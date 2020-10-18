#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pscheck.h"
#include "ping_list.h"

int check_avg(chunk_list* chunks, ping_chunk* ping);
int check_avg_interval(chunk_list* chunks, ping_chunk* ping);
int check_avg_gradient(chunk_list* chunks, ping_chunk* ping);
int check_avg(chunk_list* chunks, ping_chunk* ping);
void destroy_tmp_chunklist(chunk_list* cl);

void check_stats(chunk_list* chunks, ping_chunk* ping) {
	if(chunks->global_stats != NULL) {
		if(check_avg(chunks, ping)) {
			report(chunks, "avg", ping);
		}
		if(check_avg_interval(chunks, ping)) {
			report(chunks, "min-max", ping);
		}
		if(check_avg_gradient(chunks, ping)) {
			report(chunks, "grad", ping);
		}
	}
}


void report(chunk_list* chunks, char* out_range_stats, ping_chunk* ping) {
	printf("%d chunk %s is out of range\n", chunks->size, out_range_stats);
	if(!strncmp(out_range_stats, "avg", strlen(out_range_stats))) {
		printf("current avg:\t\t%f\n", chunks->global_stats->avg);
		printf("last ping avg:\t\t%f\n", ping->chunk_stats->avg);
	} else if (!strncmp(out_range_stats, "min-max", strlen(out_range_stats))){
		printf("current min-max:\t\t%f - %f\n", chunks->tail->chunk_stats->min, chunks->tail->chunk_stats->max);
		printf("last ping min-max:\t\t%f - %f\n", ping->chunk_stats->min, ping->chunk_stats->max);
	} else if(!strncmp(out_range_stats, "grad", strlen(out_range_stats))) {
		float tail_avg = (chunks->tail->chunk_stats->avg);
		float ping_avg = (ping->chunk_stats->avg);
		float m = ping_avg - tail_avg;
		printf("last gradient:\t\t\t%f\n", m);
		chunk_list* lasts = sublist2end(chunks, chunks->size - 3);
		ping_chunk* ptr = lasts->head;
		while(ptr->next != NULL) {
			m = ptr->next->chunk_stats->avg - ping->chunk_stats->avg;
			ptr = ptr->next;
		}
		lasts->destroy(lasts);
		printf("last 3 gradients:\t\t%f\n", m);
	}
}


int check_avg_interval(chunk_list* chunks, ping_chunk* ping) {
	float ping_interval = ((float) (ping->chunk_stats->max - ping->chunk_stats->min)) / ping->size;
	float chunks_interval = ((float) (chunks->tail->chunk_stats->max - chunks->tail->chunk_stats->min)) / chunks->tail->size;
	if(ping_interval - chunks_interval > 0.25f) return 1;
	if(ping_interval > chunks_interval) {
		chunk_list* lasts = sublist2end(chunks, chunks->size - 4);
		ping_chunk* ptr = lasts->head;
		int flag = 1;
		while(ptr != NULL) {
			float last_interval = ((float) (ptr->chunk_stats->max - ptr->chunk_stats->min)) / ptr->size;
			flag = flag && ping_interval > last_interval;
			ptr = ptr->next;
		}
		lasts->destroy(lasts);
		return flag;
	}
	return 0;
}


int check_avg_gradient(chunk_list* chunks, ping_chunk* ping) {
	float tail_avg = (chunks->tail->chunk_stats->avg);
	float ping_avg = (ping->chunk_stats->avg);
	float m = ping_avg - tail_avg;
	if(m > 1) return 1;
	else if(m > 0.33f){
		chunk_list* lasts = sublist2end(chunks, chunks->size - 4);
		ping_chunk* ptr = lasts->head;
		int flag = 1;
		while(ptr->next != NULL) {
			m = ptr->next->chunk_stats->avg - ping->chunk_stats->avg;
			flag = flag && m > 0.33f;
			ptr = ptr->next;
		}
		lasts->destroy(lasts);
		return flag;
	}
	return 0;
}



int check_avg(chunk_list* chunks, ping_chunk* ping) {
	if(ping->chunk_stats->avg - chunks->global_stats->avg > 0.5f) {
		return 1;
	} else if(ping->chunk_stats->avg > chunks->global_stats->avg) {
		chunk_list* lasts = sublist2end(chunks, chunks->size - 3);
		ping_chunk* ptr = lasts->head;
		int flag = 1;
		while(ptr != NULL) {
			flag = flag && ping->chunk_stats->avg > ptr->chunk_stats->avg;
			ptr = ptr->next;
		}
		lasts->destroy(lasts);
		return flag;
	}
	return 0;
}


void destroy_tmp_chunklist(chunk_list* cl) {
	free(cl->global_stats);
	free(cl);
}