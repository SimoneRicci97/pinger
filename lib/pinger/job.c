#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <ping_list.h>
#include <chout.h>
#include <string_utils.h>
#include <pscheck.h>
#include <job.h>

//#define _PINGER_MOCK

#ifdef _PINGER_MOCK
	#define PING "../pingmock/pingmock"
#else
	#define PING "/bin/ping"
#endif

char* extract_ping_interval(const char* out_line);
ping_stats* extract_ping_stats(char* stats_line);
int extract_lost_packets(char* loss_line);


void* doping_r(void* t_args) {
	pinger_thread_args* args = (pinger_thread_args*) t_args;
	pthread_mutex_lock(&args->hosts_data->mutex);
	chunk_list* chunks = args->hosts_data->get(args->hosts_data, args->host, strlen(args->host));//new_chunk_list();
	pthread_mutex_unlock(&args->hosts_data->mutex);
	
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
			size_t chout_size;
			char** ping_out = read_child_output(filedes[0], &chout_size);
			if(!ping_out || chout_size != atoi(args->n_ping) + 5) {
				for(int i=0; i<chout_size; i++) {
					if(ping_out[i]) free(ping_out[i]);
				}
				free(ping_out);
				return NULL;
			} 
			int i=1;
			ping_chunk* plist = new_ping_chunk(atoi(args->n_ping));
			while(i <= atoi(args->n_ping)) {
				char* string_interval = extract_ping_interval(ping_out[i]);
				if(string_interval) {
					plist->add(plist, atof(string_interval));
					free(string_interval);
				}
				i++;
			}
			i = 0;
			plist->chunk_stats = extract_ping_stats(ping_out[atoi(args->n_ping) + 4]);
			plist->chunk_stats->loss = extract_lost_packets(ping_out[atoi(args->n_ping) + 3]);
			pthread_mutex_lock(&chunks->mutex);
			#ifndef _PINGER_MOCK
			check_stats(chunks, plist);
			#endif
			chunks->add(chunks, plist);
			pthread_mutex_unlock(&chunks->mutex);
			//plist->destroy(plist);
			for(int i=0; i<atoi(args->n_ping) + 5; i++) {
				if(ping_out[i]) free(ping_out[i]);
			}
			free(ping_out);
		} else {
			printf("%d) child with pid %d exited with status %d\n", chunks->size, p, pingStatus);
		}
	} else {
		while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
		close(filedes[0]);
		execv(PING, ping_argv);
	}
	return NULL;
}


int extract_lost_packets(char* loss_line) {
	char* transmitted = pattern_substring(loss_line, "", " packets transmitted");
	char* received = pattern_substring(loss_line, "transmitted, ", " received,");
	if(transmitted && received) {
		int ftransmitted = atoi(transmitted);
		int freceived = atoi(received);
		free(transmitted);
		free(received);
		return ftransmitted - freceived;
	} 
	return 0;
}


ping_stats* extract_ping_stats(char* stats_line) {
	ping_stats* pstats = malloc(sizeof(ping_stats));
	char* stats = pattern_substring(stats_line, " = ", " ms");
	if(stats) {
		int values_size = 0;
		char** values = strspltc(stats, '/', &values_size);
		// printf("\n\n%s\n\n", stats);
		// printf("\n\n%s\n\n", values[0]);
		pstats->min = atof(values[0]);
		// printf("\n\n%s\n\n", values[1]);
		pstats->avg = atof(values[1]);
		// printf("\n\n%s\n\n", values[2]);
		pstats->max = atof(values[2]);
		// printf("\n\n%s\n\n", values[3]);
		pstats->stdev = atof(values[3]);
		free(stats);
		for(int i=0; i<values_size; i++) free(values[i]);
		free(values);
	}
	return pstats;
}


char* extract_ping_interval(const char* out_line) {
	return pattern_substring(out_line, "time=", " ms");
}
