#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "ping_list.h"

#ifndef TEST
#define PING "/bin/ping"
#endif

#ifdef TEST
#define PING "./test"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define N_PING 10
#define BUFFER_SIZE 1024

int isIpAddress(char const* arg);
int check_argv(int argc, char const* argv[]);
char** read_ping_output(int fd);
size_t sum_lens(char** s, int len);
char** format_ping_output(char* s);
int count_token(char* s);
char* to_single_string(char** s1, int len);
char* extract_ping_interval(const char* out_line);


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
		char** ping_out = read_ping_output(filedes[0]);
		int i=1;
		ping_time_list* plist = new_ping_list();
		while(i <= N_PING) {
			plist->add(plist, atof(extract_ping_interval(ping_out[i])));
			i++;
		}
		ping_time* ptr = plist->head;
		i = 0;
		while(ptr != NULL) {
			printf("interval %d: %f\n\n",i, ptr->interval);
			ptr = ptr->next;
			i++;
		}
		printf("avg: %.4f", plist->avg);
		printf("child with pid %d exited with status %d\n", p, pingStatus);
	} else {
		while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
		close(filedes[0]);
		execv(PING, ping_argv);
	}
	return 0;
}


char* extract_ping_interval(const char* out_line) {
	char* s = strndup(out_line, strlen(out_line));
	char* index = strstr(s, "time=");
	char* index1 = strstr(s, " ms" );
	index += strlen("time=")  * sizeof(char);
	char* s1 = strndup(index, (index1 - index) / sizeof(char));
	printf("%s\n", s1);
	return s1;
}


char** format_ping_output(char* s) {
	char** lines;
	int n_lines = count_token(s);
	lines = malloc((n_lines + 1) * sizeof(char*));
	for(int i=0; i<n_lines; i++) {
		lines[i] = malloc(128 * sizeof(char));
	}
	int i = 0;
	size_t index = strcspn(s, "\n");
	while(strlen(s) > 0 && s[index] == '\n') {
		strncpy(lines[i], s, index);
		printf("%s\n", lines[i]);
		i++;
		s = &s[index+1];
		index = strcspn(s, "\n");
	}
	lines[i] = NULL;
	return lines;
}


char* to_single_string(char** s1, int len) {
	char* s = malloc(sum_lens(s1, len) * sizeof(char));
	int copied = 0;
	for(int i=0; i<len; i++) {
		strncpy(&s[copied], s1[i], strlen(s1[i]));
		copied += strlen(s1[i]);
	}
	return s;
}


int count_token(char* s) {
	int i = 0;
	int count = 0;
	while(s[i] != '\0') {
		if(s[i] == '\n') count ++;
		i++;
	}
	return count;
}


size_t sum_lens(char** s, int len) {
	size_t sum = 0;
	for(int i=0; i<len; i++) {
		sum += strlen(s[i]);
	}
	return sum;
}

char** read_ping_output(int fd) {
	char buffer[BUFFER_SIZE];
	char** out = malloc(128 * sizeof(char*));
	int count = 0;
	ssize_t r = read(fd, buffer, BUFFER_SIZE);
	while(r != 0) {
		if(r == -1 && errno != EINTR) {
			printf("errno: %d\nEINTR: %d\n", errno, EINTR);
			perror("read");
			exit(1);
		} else {
			out[count] = malloc(BUFFER_SIZE * sizeof(char));
			strncpy(out[count], buffer, strlen(buffer));
			count++;
		}
		r = read(fd, buffer, BUFFER_SIZE);
	}
	out[count] = NULL;
	close(fd);
	return format_ping_output(to_single_string(out, count));
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