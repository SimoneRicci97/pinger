#include <stdio.h>
#include <stdlib.h>
#include "ping_list.h"

void _add(ping_time_chunk* l, float interval);
void _destroy(ping_time_chunk* l);
void _add_chunk(chunk_list* cl, ping_time_chunk* chunk);
void _destroy_chunk_list(chunk_list* cl);
float __min__(float f1, float f2);
float __max__(float f1, float f2);
chunk_list* new_chunk_list();

ping_time_chunk* new_ping_chunk(long chunk_size) {
	ping_time_chunk* l = malloc(sizeof(ping_time_chunk));
	l->values = malloc(sizeof(ping_time) * chunk_size);
	l->size = 0;
	l->index = 0;
	l->add = _add;
	l->destroy = _destroy;
	return l;
}

void _add(ping_time_chunk* l, float interval) {
	ping_time* item = malloc(sizeof(ping_time));
	item->next = NULL;
	item->prec = NULL;
	item->interval = interval;
	l->values[l->index].interval = interval;
	l->index++;
	l->size++;
	l->chunk_stats = NULL;
}


void _destroy(ping_time_chunk* l) {
	if(l == NULL) return;
	if(l->values != NULL) free(l->values);
	if(l->chunk_stats != NULL) free(l->chunk_stats);
	free(l);
}


void _add_chunk(chunk_list* cl, ping_time_chunk* chunk) {
	if(cl->head == NULL) cl->head = chunk;
	chunk->next = NULL;
	chunk->prec = cl->tail;
	if(cl->tail != NULL) {
		cl->tail->next = chunk;
	} 
	cl->tail = chunk;
	if(cl->global_stats != NULL) {
		cl->global_stats->avg = \
			((cl->global_stats->avg * cl->size) + chunk->chunk_stats->avg) / \
			(cl->size + 1);
		cl->global_stats->min = __min__(cl->global_stats->min, chunk->chunk_stats->min);
		cl->global_stats->max = __max__(cl->global_stats->max, chunk->chunk_stats->max);
		cl->global_stats->stdev = chunk->chunk_stats->stdev;
	} else {
		ping_stats* stats = malloc(sizeof(ping_stats));
		stats->avg = chunk->chunk_stats->avg;
		stats->min = chunk->chunk_stats->min;
		stats->max = chunk->chunk_stats->max;
		stats->stdev = chunk->chunk_stats->stdev;
		cl->global_stats = stats;
	}
	cl->size++;
}


void _destroy_chunk_list(chunk_list* cl) {
	ping_time_chunk* ptr = cl->head;
	while(ptr != NULL) {
		ptr->prec = NULL;
		cl->head = ptr->next;
		ptr->destroy(ptr);
		ptr = cl->head;
	}
	if(cl->global_stats != NULL) {
		free(cl->global_stats);
	}
	free(cl);
}

float __min__(float f1, float f2) {
	return f1 < f2 ? f1 : f2;
}

float __max__(float f1, float f2) {
	return f1 > f2 ? f1 : f2;
}

chunk_list* new_chunk_list() {
	chunk_list* cl = malloc(sizeof(chunk_list));
	cl->head = NULL;
	cl->tail = NULL;
	cl->size = 0;
	cl->global_stats = NULL;
	cl->add = _add_chunk;
	cl->destroy = _destroy_chunk_list;
	return cl;
}

// int main4test(int argc, char const *argv[])
// {
// 	ping_time_chunk* l = new_ping_chunk(10);
// 	float f = 1.02;
// 	for(int i=0; i<10; i++) {
// 		l->add(l, f);
// 		f = f + 1.f;
// 	}
// 	int i = 0;
// 	for(int i=0; i<l->size; i++) {
// 		ping_time ptr = l->values[i];
// 		printf("%d: %.2f\n", i, ptr.interval);
// 		i++;
// 	}
// 	printf("printed: %d\nsize: %u\n", i, l->size);
// 	return 0;
// }