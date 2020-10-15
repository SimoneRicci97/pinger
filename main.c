#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "lib/ping_list.h"
#include "lib/chout.h"
#include "lib/string_utils.h"
#include "lib/htable.h"
#include "lib/configuration.h"
#include "lib/pscheck.h"
#include "pinger_config.h"

#ifdef _PINGER_MOCK
	#define PING "./pingmock"
#else
	#define PING "/bin/ping"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MAX_HOST "pinger.maxhost"
#define NPING "pinger.default.nping"

typedef struct _pinger_args {
	char** hosts;
	char* n_ping;
	int size;
} pinger_args;

void usage(char* prog_name, char* message);
int isIpAddress(char* arg);
pinger_args* check_argv(int argc, char* argv[], htable* conf);
char* extract_ping_interval(const char* out_line);
ping_stats* extract_ping_stats(char* stats_line);
int extract_lost_packets(char* loss_line);
void output(chunk_list* chunks);
void version(char* prog_name);


int main(int argc, char *argv[]) {
	char* conf_path[3] = {"../", argv[0], ".conf"};
	char* conf_path_s = string_builder(conf_path, 3, NULL);
	htable* conf = load_configuration(conf_path_s);
	free(conf_path_s);
	pinger_args* args = check_argv(argc, argv, conf);

	if(args == NULL) {
		exit(1);
	}
	chunk_list* chunks = new_chunk_list();
	int q = 0;
	for(int k=0; k<10; k++) {
		char* ping_argv[5] = {PING, args->hosts[q], "-c", args->n_ping, NULL};
		int filedes[2];
		if (pipe(filedes) == -1) {
			perror("pipe");
			exit(1);
		}
		int pingStatus;
		int p = fork();
		if(p > 0) {
			close(filedes[1]);
			waitpid(p, &pingStatus, 0);
			if(pingStatus == 0) {
				char** ping_out = read_child_output(filedes[0]);
				int i=1;
				ping_time_chunk* plist = new_ping_chunk(atoi(args->n_ping));
				while(i <= atoi(args->n_ping)) {
					char* string_interval = extract_ping_interval(ping_out[i]);
					plist->add(plist, atof(string_interval));
					free(string_interval);
					i++;
				}
				i = 0;
				plist->chunk_stats = extract_ping_stats(ping_out[atoi(args->n_ping) + 4]);
				plist->chunk_stats->loss = extract_lost_packets(ping_out[atoi(args->n_ping) + 3]);
				check_stats(chunks, plist);
				chunks->add(chunks, plist);
				//plist->destroy(plist);
				for(int i=0; i<atoi(args->n_ping) + 5; i++) {
					free(ping_out[i]);
				}
				free(ping_out);
			}
			printf("%d) child with pid %d exited with status %d\n", k, p, pingStatus);
			sleep(1);
		} else {
			//chunks->destroy(chunks);
			while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
			close(filedes[0]);
			execv(PING, ping_argv);
		}
	}
	output(chunks);
	chunks->destroy(chunks);
	conf->destroy(conf);
	for(int i=0; i<args->size; i++) {
		free(args->hosts[i]);
	}
	free(args->hosts);
	free(args);
	return 0;
}

void output(chunk_list* chunks) {
	if(chunks->head != NULL) {
		ping_time_chunk* ptr = chunks->head;
		printf("min/ avg/ max/ sdev\n");
		while(ptr != NULL) {
			printf("%.3f/%.3f/%.3f/%.3f\n", ptr->chunk_stats->min, ptr->chunk_stats->avg, ptr->chunk_stats->max, ptr->chunk_stats->stdev);
			ptr = ptr->next;
		}
		printf("-----------------------\n");
		printf("%.3f/%.3f/%.3f/%.3f\n", chunks->global_stats->min, chunks->global_stats->avg, chunks->global_stats->max, chunks->global_stats->stdev);
	} else {
		printf("### No pings ###\n");
	}
	
}


int extract_lost_packets(char* loss_line) {
	char* transmitted = pattern_substring(loss_line, "", " packets transmitted");
	char* received = pattern_substring(loss_line, "transmitted, ", " received,");
	float ftransmitted = atoi(transmitted);
	float freceived = atoi(received);
	free(transmitted);
	free(received);
	return ftransmitted - freceived;
}


ping_stats* extract_ping_stats(char* stats_line) {
	ping_stats* pstats = malloc(sizeof(ping_stats));
	char* stats = pattern_substring(stats_line, " = ", " ms");
	char* value = strtok(stats, "/");
	pstats->min = atof(value);
	value = strtok(NULL, "/");
	pstats->avg = atof(value);
	value = strtok(NULL, "/");
	pstats->max = atof(value);
	value = strtok(NULL, "/");
	pstats->stdev = atof(value);
	free(stats);
	return pstats;
}


char* extract_ping_interval(const char* out_line) {
	return pattern_substring(out_line, "time=", " ms");
}


int isIpAddress(char* arg) {
	char ipBytes[4];
	int i=0;
	while(i < strlen(arg)) {
		int j = i;
		int b = 0;
		while(j < strlen(arg) && arg[j] != '.') {
			ipBytes[b++] = arg[j++];
		}
		ipBytes[b] = '\0';
		int intByte = atoi(ipBytes);
		if(!(intByte >= 0 && intByte < 256)) return 0;
		i = j + 1;
	}

	return 1;
}


void version(char* prog_name) {
	fprintf(stderr, "%s version: %d.%d.%d\n", prog_name, PINGER_VERSION_MAJOR, PINGER_VERSION_MINOR, PINGER_VERSION_PATCH);
	fprintf(stderr, "\n");
}


void usage(char* prog_name, char* message) {
	if(message != NULL) {
		fprintf(stderr, "%s\n", message);
	}
	version(prog_name);
	fprintf(stderr, "Use %s -l host_list... [-ch]\n", prog_name);
	fprintf(stderr, "-c\t| number of ping\n");
	fprintf(stderr, "-h\t| print this message\n");
	fprintf(stderr, "\nhost_list:\t host comma separated list.\n");
}


pinger_args* check_argv(int argc, char* argv[], htable* conf) {
	if(argc <= 1) return NULL;
	const char* optstring = "c:hv";
	pinger_args* args = malloc(sizeof(pinger_args));
	args->size = 0;
	char optc;
	int carg = 0;
	while((optc = getopt(argc, argv, optstring)) != -1) {
		switch(optc) {
			case 'c': {
				args->n_ping = optarg;
				carg = 1;
			} break;
			case 'h': {
				usage(argv[0], NULL);
				free(args);
				return NULL;
			} break;
			case 'v': {
				version(argv[0]);
				free(args);
				return NULL;
			} break;
			default: {
				char* errmessage = string_concat(optarg, " is not a valid option.\n");
				usage(argv[0], errmessage);
				free(errmessage);
				free(args->hosts);
				free(args);
				return NULL;
			}
		} 
	}
	if(optind > 0 && argv[optind] != NULL) {
		args->hosts = strspltc(argv[optind], ',', &args->size);
	} else {
		usage(argv[0], "Missing host argument.");
		return NULL;
	}
	if(!carg) args->n_ping = get(conf, NPING, strlen(NPING));
	for(int i=1; i<argc; i++) {
		if(!isIpAddress(argv[i])) {
			char* errmessage = string_concat(argv[i], " is malformed.\n");
			usage(argv[0], errmessage);
			free(errmessage);
			return NULL;
		}
	}
	return args;
}