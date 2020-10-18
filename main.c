#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <getopt.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <pthread.h>


#include "chout.h"
#include "string_utils.h"
#include "htable.h"
#include "configuration.h"

#include "ping_list.h"
#include "pscheck.h"
#include "job.h"
#include "pinger_config.h"

#include <pthreadpool.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MAX_HOST "pinger.maxhost"
#define NPING "pinger.default.nping"
#define RPING "pinger.ping.runs"

typedef struct _pinger_args {
	char** hosts;
	char* n_ping;
	int runs;
	int size;
} pinger_args;

void usage(char* prog_name, char* message);
int isIpAddress(char* arg);
pinger_args* check_argv(int argc, char* argv[], htable* conf);
void version(char* prog_name);

void chunkdestroy(void* chunklist);
void strdestroy(void* str);


int main(int argc, char *argv[]) {
	char* conf_path[3] = {"../", argv[0], ".conf"};
	char* conf_path_s = string_builder(conf_path, 3, NULL);

	htable* conf = load_configuration(conf_path_s);
	free(conf_path_s);
	pinger_args* args = check_argv(argc, argv, conf);

	if(args == NULL) {
		exit(1);
	}

	htable* hosts = new_htable(strdestroy, chunkdestroy);
	for(int i=0; i<args->size; i++) {
		hosts->put(hosts, args->hosts[i], new_chunk_list(), strlen(args->hosts[i]));
	}
	pthreadpool_t* tp = new_threadpool(4, 1);
	pinger_thread_args* t_args = malloc(sizeof(pinger_thread_args) * args->size);
	for(int t=0; t<args->size; t++) {
		t_args[t].host = args->hosts[t];
		t_args[t].n_ping = args->n_ping;
		t_args[t].runs = args->runs;
		t_args[t].hosts_data = hosts;
		tp->add_task(tp, t_doping, &t_args[t]);
	}
	tp->start(tp);
	sleep(10);
	tp->stop(tp);
	free(t_args);

	conf->destroy(conf);
	for(int i=0; i<args->size; i++) {
		free(args->hosts[i]);
	}
	free(args->hosts);
	free(args);
	return 0;
}


void chunkdestroy(void* chunklist) {
	((chunk_list*) chunklist)->destroy(((chunk_list*) chunklist));
}


void strdestroy(void* str) {
	free(str);
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
	fprintf(stderr, "-c\t| number of ping packets sent\n");
	fprintf(stderr, "-r\t| number of ping runs\n");
	fprintf(stderr, "-h\t| print this message\n");
	fprintf(stderr, "\nhost_list:\t host comma separated list.\n");
}


pinger_args* check_argv(int argc, char* argv[], htable* conf) {
	if(argc <= 1) return NULL;
	const char* optstring = "c:r:hv";
	pinger_args* args = malloc(sizeof(pinger_args));
	args->size = 0;
	char optc;
	int c_arg = 0;
	int r_arg = 0;
	while((optc = getopt(argc, argv, optstring)) != -1) {
		switch(optc) {
			case 'c': {
				args->n_ping = optarg;
				c_arg = 1;
			} break;
			case 'r': {
				args->runs = atoi(optarg);
				r_arg = 1;
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
		if(args->size > atoi((char*) conf->get(conf, MAX_HOST, strlen(MAX_HOST)))) {
			fprintf(stderr, "Warning: passed a number of hosts to ping greater than values of %s.\n" \
				"Note that pinger generate a thread for each host passed, so if you think your machine " \
				"is able to bear the load of %d threads, you can increase the value of %s in pinger.conf.\n", \
				MAX_HOST, args->size, MAX_HOST);
			return NULL;
		}
	} else {
		usage(argv[0], "Missing host argument.");
		return NULL;
	}
	if(!c_arg) args->n_ping = conf->get(conf, NPING, strlen(NPING));
	if(!r_arg) args->runs = atoi((char*) conf->get(conf, RPING, strlen(RPING)));

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