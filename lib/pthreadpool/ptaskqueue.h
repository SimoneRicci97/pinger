#ifndef __TASK_QUEUE__
#define __TASK_QUEUE__ value

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptask.h"

typedef struct _ptaskq_item {
	ptask_t* task;
	struct _ptaskq_item* next;
	struct _ptaskq_item* prec;
} ptaskq_item;


typedef struct _ptaskq {
	ptaskq_item* head;
	ptaskq_item* tail;
	ptaskq_item* it;
	int size;
	short circle;
	void (*add) (struct _ptaskq*, ptask_t*);
	ptaskq_item* (*next) (struct _ptaskq* queue, int* term);
	void (*destroy) (struct _ptaskq*);
	pthread_mutex_t mutex;
	pthread_cond_t empty_semaphore;
} ptaskq_t;

ptaskq_item* new_ptaskq_item(ptask_t* ptask);

ptaskq_t* new_ptaskq(short circle);


#endif
