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
} exitq_t;


exitq_t* new_retvalq();

exit_item_t* pretval_get(exitq_t* q, int* term);

void pretval_insert(exitq_t* q, void* extstatus);

void pretval_destroy(exitq_t* q);



#endif
