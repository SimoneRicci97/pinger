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
ping_time_chunk* ping_time_chunk_clone(ping_time_chunk* toclone);

ping_time_chunk* new_ping_chunk(long chunk_size) {
	ping_time_chunk* l = malloc(sizeof(ping_time_chunk));
	l->values = malloc(sizeof(ping_time) * chunk_size);
	l->size = 0;
	l->index = 0;
	l->add = _add;
	l->destroy = _destroy;
	l->next = NULL;
	l->prec = NULL;
	return l;
}

ping_time_chunk* ping_time_chunk_clone(ping_time_chunk* toclone) {
	ping_time_chunk* clone = new_ping_chunk(toclone->size);
	for(int i= 0; i<toclone->index; i++) {
		clone->add(clone, toclone->values[i].interval);
	}
	clone->chunk_stats = malloc(sizeof(ping_stats));
	clone->chunk_stats->min = toclone->chunk_stats->min;
	clone->chunk_stats->max = toclone->chunk_stats->max;
	clone->chunk_stats->avg = toclone->chunk_stats->avg;
	clone->chunk_stats->stdev = toclone->chunk_stats->stdev;
	clone->chunk_stats->loss = toclone->chunk_stats->loss;
	return clone;
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
	if(l == NULL) {
		return;
	}
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

chunk_list* sublist2end(chunk_list* cl, int start) {
	return sublist(cl, start >= 0 ? start : 0, cl->size);
}

chunk_list* sublist2start(chunk_list* cl, int end) {
	return sublist(cl, 0, cl->size);
}

chunk_list* sublist(chunk_list* cl, int start, int end) {
	chunk_list* clone = new_chunk_list();
	if(start < 0 || end > cl->size || end <= start) return clone;
	ping_time_chunk* ptr = cl->head;
	for(int i=0; i < start; i++) ptr = ptr->next;

	while(start < end && ptr != NULL) {
		clone->add(clone, ping_time_chunk_clone(ptr));
		ptr = ptr->next;
		start++;
	}
	return clone;
}