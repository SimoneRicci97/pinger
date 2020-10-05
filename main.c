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

int isIpAddress(char const* arg);
int check_argv(int argc, char const* argv[]);
char* extract_ping_interval(const char* out_line);
ping_stats* extract_ping_stats(char* stats_line);
char* pattern_substring(const char* s, char* begin_pattern, char* end_pattern);
int extract_lost_packets(char* loss_line);


int main(int argc, char const *argv[]) {
	// char* targetHost = argv[1];
	int bad = check_argv(argc, argv);
	int pingStatus;
	if(bad > 0) {
		printf("ERROR: %s is malformed\n", argv[bad]);
	}
	
	char** ping_argv = malloc(4 * sizeof(char*));
	ping_argv[0] = PING;
	ping_argv[1] = strndup(argv[1], strlen(argv[1]));
	ping_argv[2] = "-c";
	ping_argv[3] = TOSTRING(N_PING);
	ping_argv[4] = NULL;
	
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
		printf("read all\n");
		int i=1;
		ping_time_chunk* plist = new_ping_chunk(N_PING);
		while(i <= N_PING) {
			plist->add(plist, atof(extract_ping_interval(ping_out[i])));
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
		printf("child with pid %d exited with status %d\n", p, pingStatus);
	} else {
		while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
		close(filedes[0]);
		execv(PING, ping_argv);
	}
	return 0;
}


int extract_lost_packets(char* loss_line) {
	char* transmitted = pattern_substring(loss_line, "", " packets transmitted");
	char* received = pattern_substring(loss_line, "transmitted, ", " received,");
	return atoi(transmitted) - atoi(received);
}


ping_stats* extract_ping_stats(char* stats_line) {
	ping_stats* pstats = malloc(sizeof(ping_stats));
	char* stats = pattern_substring(stats_line, " = ", " ms");
	printf("stats substring: %s\n", stats);
	char* value = strtok(stats, "/");
	pstats->min = atof(value);
	value = strtok(NULL, "/");
	pstats->avg = atof(value);
	value = strtok(NULL, "/");
	pstats->max = atof(value);
	value = strtok(NULL, "/");
	pstats->stdev = atof(value);
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
	return s1;
}


int isIpAddress(char const* arg) {
	char* ipBytes;
	int i=0;
	while(i < strlen(arg)) {
		int j = i;
		int b = 0;
		ipBytes = malloc(sizeof(char) * 3);
		while(j < strlen(arg) && arg[j] != '.') {
			ipBytes[b++] = arg[j++];
		}
		ipBytes[b] = '\0';
		int intByte = atoi(ipBytes);
		if(!(intByte >= 0 && intByte < 256)) return 0;
		free(ipBytes);
		i = j + 1;
	}

	return 1;
}


int check_argv(int argc, char const* argv[]) {
	if(argc <= 1) return -1;
	int checkResult = 0;
	for(int i=1; i<argc; i++) {
		if(!isIpAddress(argv[i])) return i;
	}
	return checkResult;
}