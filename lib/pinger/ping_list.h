#ifndef __PING_CHUNK__
#define __PING_CHUNK__ 

#include <pthread.h>

typedef struct _ping_stats {
	float avg;
	float min;
	float max;
	float stdev;
	int loss;
} ping_stats;

typedef struct _ping_time_chunk {
	float* values;
	long index;
	long size;
	ping_stats* chunk_stats;
	void (*add) (struct _ping_time_chunk*, float);
	void (*destroy) (struct _ping_time_chunk*);
	struct _ping_time_chunk* next;
	struct _ping_time_chunk* prec;
} ping_chunk;


typedef struct _chunk_list {
	ping_chunk* head;
	ping_chunk* tail;
	int size;
	ping_stats* global_stats;
	void (*add) (struct _chunk_list*, ping_chunk*);
	void (*destroy) (struct _chunk_list*);
	pthread_mutex_t mutex;
} chunk_list;

ping_chunk* new_ping_chunk(long chunk_size);

chunk_list* new_chunk_list();

chunk_list* sublist(chunk_list* cl, int start, int end);

chunk_list* sublist2end(chunk_list* cl, int start);

chunk_list* sublist2start(chunk_list* cl, int end);

#endif
