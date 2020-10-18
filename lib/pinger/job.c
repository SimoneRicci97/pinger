#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include "ping_list.h"
#include "chout.h"
#include "string_utils.h"
#include "pscheck.h"
#include "job.h"

#define PINGER_OUT_PREFIX ".pinger.log"

#define _PINGER_MOCK

#ifdef _PINGER_MOCK
	#define PING "../pingmock/pingmock"
#else
	#define PING "/bin/ping"
#endif

char* extract_ping_interval(const char* out_line);
ping_stats* extract_ping_stats(char* stats_line);
int extract_lost_packets(char* loss_line);
void output(char* host, chunk_list* chunks);


void* t_doping(void* t_args) {
	pinger_thread_args* args = (pinger_thread_args*) t_args;
	printf("pinging host %s\n", args->host);
	chunk_list* chunks = args->hosts_data->get(args->hosts_data, args->host, strlen(args->host));//new_chunk_list();
	pthread_mutex_lock(&chunks->mutex);
	if(args->runs >= 0 && chunks->size >= args->runs) {
		pthread_mutex_unlock(&chunks->mutex);
		return NULL;
	}
	pthread_mutex_unlock(&chunks->mutex);
	printf("running %dth ping of %d\n", chunks->size, args->runs);
	char* ping_argv[5] = {PING, args->host, "-c", args->n_ping, NULL};
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
			ping_chunk* plist = new_ping_chunk(atoi(args->n_ping));
			while(i <= atoi(args->n_ping)) {
				char* string_interval = extract_ping_interval(ping_out[i]);
				plist->add(plist, atof(string_interval));
				free(string_interval);
				i++;
			}
			i = 0;
			plist->chunk_stats = extract_ping_stats(ping_out[atoi(args->n_ping) + 4]);
			plist->chunk_stats->loss = extract_lost_packets(ping_out[atoi(args->n_ping) + 3]);
			pthread_mutex_lock(&chunks->mutex);
			check_stats(chunks, plist);
			chunks->add(chunks, plist);
			pthread_mutex_unlock(&chunks->mutex);
			//plist->destroy(plist);
			for(int i=0; i<atoi(args->n_ping) + 5; i++) {
				free(ping_out[i]);
			}
			free(ping_out);
		}
		printf("%d) child with pid %d exited with status %d\n", chunks->size, p, pingStatus);
	} else {
		//chunks->destroy(chunks);
		while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
		close(filedes[0]);
		execv(PING, ping_argv);
	}
	output(args->host, chunks);
	//chunks->destroy(chunks);
	return NULL;
}


void output(char* host, chunk_list* chunks) {
	pthread_mutex_lock(&chunks->mutex);
	printf("creating log file for %s\n", host);
	char* logpatharr[2] = {host, PINGER_OUT_PREFIX};
	char* logpath = string_builder(logpatharr, 2, NULL);
	FILE* endout = fopen(logpath, "w");
	free(logpath);
	if(chunks->head != NULL) {
		ping_chunk* ptr = chunks->head;
		fprintf(endout, "min/ avg/ max/ sdev\n");
		while(ptr != NULL) {
			fprintf(endout, "%.3f/%.3f/%.3f/%.3f\n", ptr->chunk_stats->min, ptr->chunk_stats->avg, ptr->chunk_stats->max, ptr->chunk_stats->stdev);
			ptr = ptr->next;
		}
		fprintf(endout, "-----------------------\n");
		fprintf(endout, "%.3f/%.3f/%.3f/%.3f\n", chunks->global_stats->min, chunks->global_stats->avg, chunks->global_stats->max, chunks->global_stats->stdev);
	} else {
		fprintf(stderr, "### No pings ###\n");
	}
	fclose(endout);
	pthread_mutex_unlock(&chunks->mutex);
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
