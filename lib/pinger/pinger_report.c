#include <pinger_report.h>
#include <htable.h>
#include <ping_list.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string_utils.h>
#include <uttime.h>
#include <math.h>
#include <sys/stat.h>

void _output(char* outdir, char* host, chunk_list* chunks);


void do_report(void* report_args) {
	htable* hosts_chunks = ((report_args_t*) report_args)->hosts;
	pthread_mutex_lock(&hosts_chunks->mutex);
	char* reportout = ((report_args_t*) report_args)->reportout;
	for(int i=0; i<hosts_chunks->size; i++) {
		if(hosts_chunks->keys[i]) {
			chunk_list* chunks = hosts_chunks->get(hosts_chunks, \
				hosts_chunks->keys[i]->head->key, \
				strlen((char*) hosts_chunks->keys[i]->head->key));
			_output(reportout, (char*) hosts_chunks->keys[i]->head->key, chunks);
		}
	}
	pthread_mutex_unlock(&hosts_chunks->mutex);
}


void _output(char* outdir, char* host, chunk_list* chunks) {
	ulong instant = get_current_millis();
	char logpath[128];
	snprintf(logpath, 128, "%s/%s_%ld%s", outdir, host, instant, PINGER_OUT_PREFIX);
	FILE* endout = fopen(logpath, "w");
	char*  start = format_timestamp(chunks->tstart);
	char* end = get_formatted_datetime();
	fprintf(endout, "%s - %s\n", start, end);
	free(start);
	free(end);
	if(chunks->head != NULL) {
		ping_chunk* ptr = chunks->head;
		fprintf(endout, "min/ avg/ max/ sdev\n");
		while(ptr != NULL) {
			char* pingts = format_timestamp(ptr->ts);
			fprintf(endout, "%s\t|%.3f/%.3f/%.3f/%.3f\n", pingts, \
			 ptr->chunk_stats->min, ptr->chunk_stats->avg, ptr->chunk_stats->max, ptr->chunk_stats->stdev);
			ptr = ptr->next;
			free(pingts);
		}
		fprintf(endout, "-----------------------\n");
		fprintf(endout, "%.3f/%.3f/%.3f/%.3f\n", chunks->global_stats->min, chunks->global_stats->avg, chunks->global_stats->max, chunks->global_stats->stdev);
	} else {
		fprintf(stderr, "### No pings ###\n");
	}
	fclose(endout);
	//chmod(logpath, S_IRUSR | S_IRGRP | S_IROTH);
	pthread_mutex_unlock(&chunks->mutex);
}
