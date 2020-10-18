#ifndef __PRETVAL_QUEUE
#define __PRETVAL_QUEUE

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

typedef struct _exit_item {
	void* exit_status;
	struct _exit_item* next;
	struct _exit_item* prec;
} exit_item_t;


typedef struct _exitq {
	exit_item_t* head;
	exit_item_t* tail;
	int size;
	pthread_mutex_t mutex;
	pthread_cond_t empty_semaphore;
	void* (*get) (struct _exitq*, int*);
	void (*insert) (struct _exitq*, void*);
	void (*destroy) (struct _exitq*);
} exitq_t;


exitq_t* new_retvalq();


#endif
