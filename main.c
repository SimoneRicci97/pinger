#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "ping_list.h"
#include "chout.h"

#ifndef TEST
#define PING "/bin/ping"
#endif

#ifdef TEST
#define PING "./test"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define N_PING 10

int isIpAddress(char* arg);
int check_argv(int argc, char* argv[]);
char* extract_ping_interval(const char* out_line);
ping_stats* extract_ping_stats(char* stats_line);
char* pattern_substring(const char* s, char* begin_pattern, char* end_pattern);
int extract_lost_packets(char* loss_line);


int main(int argc, char *argv[]) {
	// char* targetHost = argv[1];
	int bad = check_argv(argc, argv);
	int pingStatus;
	if(bad > 0) {
		printf("ERROR: %s is malformed\n", argv[bad]);
	}

	char* ping_argv[5] = {PING, argv[1], "-c", TOSTRING(N_PING), NULL};
	chunk_list* chunks = new_chunk_list();
	for(int q=0; q<5; q++) {
		int filedes[2];
		if (pipe(filedes) == -1) {
			perror("pipe");
			exit(1);
		}
		int p = fork();
		if(p > 0) {
			close(filedes[1]);
			waitpid(p, &pingStatus, 0);
			char** ping_out = read_child_output(filedes[0]);
			int i=1;
			ping_time_chunk* plist = new_ping_chunk(N_PING);
			while(i <= N_PING) {
				char* string_interval = extract_ping_interval(ping_out[i]);
				plist->add(plist, atof(string_interval));
				free(string_interval);
				i++;
			}
			i = 0;
			plist->chunk_stats = extract_ping_stats(ping_out[N_PING + 4]);
			plist->chunk_stats->loss = extract_lost_packets(ping_out[N_PING + 3]);
			printf("min: %f\n", plist->chunk_stats->min);
			printf("avg: %f\n", plist->chunk_stats->avg);
			printf("max: %f\n", plist->chunk_stats->max);
			printf("stdev: %f\n", plist->chunk_stats->stdev);
			printf("loss: %d\n", plist->chunk_stats->loss);
			chunks->add(chunks, plist);
			//plist->destroy(plist);
			for(int i=0; i<N_PING + 5; i++) {
				free(ping_out[i]);
			}
			free(ping_out);
			printf("child with pid %d exited with status %d\n", p, pingStatus);
		} else {
			
			while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
			close(filedes[0]);
			execv(PING, ping_argv);
		}
	}

	ping_time_chunk* ptr = chunks->head;
	while(ptr != NULL) {
		printf("%.3f/%.3f/%.3f/%.3f\n", ptr->chunk_stats->min, ptr->chunk_stats->avg, ptr->chunk_stats->max, ptr->chunk_stats->stdev);
		ptr = ptr->next;
	}
	printf("-------------------\n");
	printf("%.3f/%.3f/%.3f/%.3f\n", chunks->global_stats->min, chunks->global_stats->avg, chunks->global_stats->max, chunks->global_stats->stdev);
	return 0;
}


int extract_lost_packets(char* loss_line) {
	printf("%s\n", loss_line);
	char* transmitted = pattern_substring(loss_line, "", " packets transmitted");
	char* received = pattern_substring(loss_line, "transmitted, ", " received,");
	float ftransmitted = atoi(transmitted);
	float freceived = atoi(received);
	free(transmitted);
	free(received);
	return ftransmitted - freceived;
}


ping_stats* extract_ping_stats(char* stats_line) {
	printf("%s\n", stats_line);
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


char* pattern_substring(const char* s, char* begin_pattern, char* end_pattern) {
	char* copy = strndup(s, strlen(s));
	char* index = strstr(copy, begin_pattern);
	char* index1 = strstr(copy, end_pattern);
	index += strlen(begin_pattern)  * sizeof(char);
	char* s1 = strndup(index, (index1 - index) / sizeof(char));
	free(copy);
	return s1;
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


int check_argv(int argc, char* argv[]) {
	if(argc <= 1) return -1;
	int checkResult = 0;
	for(int i=1; i<argc; i++) {
		if(!isIpAddress(argv[i])) return i;
	}
	return checkResult;
}