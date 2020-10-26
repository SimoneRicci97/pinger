#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptask.h"
#include "ptaskqueue.h"

#include <pthread.h>

#define CHECK_NEXT queue->it == NULL ? "NULL" : "not NULL"

void _task_add(ptaskq_t* queue, ptask_t* ptask);
ptaskq_item* _task_next(ptaskq_t* queue, int* term);
void _ptaskq_destroy(ptaskq_t* ptaskq);

ptaskq_item* new_ptaskq_item(ptask_t* ptask) {
	ptaskq_item* item = malloc(sizeof(ptaskq_item));
	item->task = ptask;
	item->next = NULL;
	item->prec = NULL;
	return item;
}

ptaskq_t* new_ptaskq(short circle) {
	ptaskq_t* ptaskq = malloc(sizeof(ptaskq_t));
	ptaskq->head = NULL;
	ptaskq->tail = NULL;
	ptaskq->size = 0;
	ptaskq->circle = circle;
	ptaskq->it = NULL;
	ptaskq->add = _task_add;
	ptaskq->next = _task_next;
	ptaskq->destroy = _ptaskq_destroy;
	pthread_mutex_init(&ptaskq->mutex, NULL);
	pthread_cond_init(&ptaskq->empty_semaphore, NULL);
	pthread_cond_init(&ptaskq->nempty_semaphore, NULL);
	return ptaskq;
}

void _task_add(ptaskq_t* queue, ptask_t* ptask) {
	if(queue == NULL || ptask == NULL) return;
	pthread_mutex_lock(&queue->mutex);
	ptaskq_item* item = new_ptaskq_item(ptask);
	if(queue->head == NULL) {
		queue->head = item;
		queue->tail = item;
		queue->it = queue->head;
	} else {
		item->prec = queue->tail;
		queue->tail->next = item;
		queue->tail = item;
		if(queue->circle) queue->tail->next = queue->head;
		//printf("added arg: %d\n", *((int*) queue->it->task->t_arg->t_arg));
	}
	queue->size++;
	pthread_cond_signal(&queue->empty_semaphore);
	pthread_mutex_unlock(&queue->mutex);
}

ptaskq_item* _task_next(ptaskq_t* queue, int* term) {
	if(queue == NULL) return NULL;
	pthread_mutex_lock(&queue->mutex);
	while(queue->size == 0 && !*term) {
		pthread_cond_wait(&queue->empty_semaphore, &queue->mutex);
	}
	//printf("Waiting end: %s - %d\n", CHECK_NEXT, *term);
	if(*term) {
		pthread_mutex_unlock(&queue->mutex);
		return NULL;
	}
	//printf("Gettin next\n");
	ptaskq_item* res = queue->it;
	//printf("Mocin it %s - %d\n", CHECK_NEXT, queue->size);
	queue->it = queue->it->next;
	if(!queue->circle) {
		queue->size--;
		pthread_cond_signal(&queue->nempty_semaphore);
	}
	//printf("releasing lock.  next is %s - %d\n", CHECK_NEXT, queue->size);
	pthread_mutex_unlock(&queue->mutex);
	//printf("returning a task: task(%d)\n", *((int*) res->task->t_arg->t_arg));
	return res;
}


void _ptaskq_destroy(ptaskq_t* ptaskq) {
	pthread_mutex_destroy(&ptaskq->mutex);
	pthread_cond_destroy(&ptaskq->empty_semaphore);
	pthread_cond_destroy(&ptaskq->nempty_semaphore);
	ptaskq_item* ptr = ptaskq->head;
	while(ptaskq->head != ptaskq->tail) {
		ptaskq->head = ptaskq->head->next;
		ptaskq->head->prec = NULL;
		ptask_destroy(ptr->task);
		free(ptr);
		ptr = ptaskq->head;
	}
	if(ptaskq->tail->task) ptask_destroy(ptaskq->tail->task);
	if(ptaskq->tail) free(ptaskq->tail);
	free(ptaskq);
}
