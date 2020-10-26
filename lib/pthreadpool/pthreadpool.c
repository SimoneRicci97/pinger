#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "pthreadpool.h"
#include "ptaskqueue.h"

typedef struct _worker_args {
	threadpool_status* status;
	int wid;
} wargs_t;

void* tpt_worker(void* args_queue);
void tpt_start(pthreadpool_t* tp);
void tpt_stop(pthreadpool_t* tp);
void tpt_wait(pthreadpool_t* tp);
void tpt_add_task(pthreadpool_t* tp, void* (*task) (void*), void* taskarg);

pthreadpool_t* new_threadpool(size_t size, int* termflag, short circle) {
	pthreadpool_t* tp = malloc(sizeof(pthreadpool_t));
	tp->workers = malloc(size * sizeof(pthread_t));
	tp->size = size;

	exitq_t* retvals = new_retvalq();
	ptaskq_t* task_q = new_ptaskq(circle);
	threadpool_status* tp_status = malloc(sizeof(threadpool_status));
	tp_status->exitworker = calloc(size, sizeof(short));
	tp_status->status = termflag;
	tp_status->ptask_q = task_q;
	tp_status->exits = retvals;
	
	tp->status = tp_status;

	tp->start = tpt_start;
	tp->add_task = tpt_add_task;
	tp->stop = tpt_stop;
	tp->wait = tpt_wait;

	pthread_mutex_init(&tp->status->status_mutex, NULL);
	return tp;
}

void tpt_add_task(pthreadpool_t* tp, void* (*task) (void*), void* taskarg) {
	ptask_t* ptask = malloc(sizeof(ptask_t));
	ptask->t_arg = malloc(sizeof(ptaskarg_t));
	ptask->t_arg->t_arg = taskarg;
	ptask->t_arg->status = tp->status->status;
	ptask->routine = task;
	tp->status->ptask_q->add(tp->status->ptask_q, ptask);
}

void tpt_stop(pthreadpool_t* tp) {
	fprintf(stderr, "Killing threads. Use ctrl+C to exit...\n");
	pthread_mutex_lock(&tp->status->status_mutex);
	*tp->status->status = 1;
	pthread_mutex_unlock(&tp->status->status_mutex);
	int exited = 0;
	while(exited < tp->size) {
		for(int i=0; i<tp->size; i++) {
			void* workerexit_v;
			pthread_cond_signal(&tp->status->ptask_q->empty_semaphore);
			if(tp->status->exitworker[i] == 1) {
				tp->status->exitworker[i] = 2;
				pthread_join(tp->workers[i], &workerexit_v);
				pthread_cancel(tp->workers[i]);
				int* workerexit = (int*) workerexit_v;
				if(*workerexit) {
					fprintf(stderr, "%dth worker failed\n", i);
				}
				free(workerexit_v);
				exited++;
			}
		}
	}
	
}


void tpt_wait(pthreadpool_t* tp) {
	fprintf(stderr, "Waiting for ends of all tasks. Use ctrl+C to exit...\n");
	pthread_mutex_lock(&tp->status->ptask_q->mutex);
	while(tp->status->ptask_q->size > 0) {
		pthread_cond_wait(&tp->status->ptask_q->nempty_semaphore, &tp->status->ptask_q->mutex);
	}
	pthread_mutex_unlock(&tp->status->ptask_q->mutex);
	*tp->status->status = 1;
	int exited = 0;
	while(exited < tp->size) {
		for(int i=0; i<tp->size; i++) {
			void* workerexit_v;
			pthread_cond_signal(&tp->status->ptask_q->empty_semaphore);
			if(tp->status->exitworker[i] == 1) {
				printf("%d exited\n", i);
				tp->status->exitworker[i] = 2;
				pthread_join(tp->workers[i], &workerexit_v);
				pthread_cancel(tp->workers[i]);
				int* workerexit = (int*) workerexit_v;
				if(*workerexit) {
					fprintf(stderr, "%dth worker failed\n", i);
				}
				free(workerexit_v);
				exited++;
			}
		}
	}
}


void tpt_start(pthreadpool_t* tp) {
	for(int i=0; i<tp->size; i++) {
		wargs_t* wargs = malloc(sizeof(wargs_t));
		wargs->wid = i;
		wargs->status = tp->status;
		int tid = pthread_create(&tp->workers[i], NULL, tpt_worker, wargs);
		if(tid) {
			perror("Creating workers");
			exit(1);
		}
	}
}


void* tpt_worker(void* __args) {
	wargs_t* wargs = (wargs_t*) __args;
	ptaskq_t* ptask_queue = wargs->status->ptask_q;
	exitq_t* exit_queue = wargs->status->exits;
	int id = wargs->wid;
	pthread_mutex_t status_mutex = wargs->status->status_mutex;
	int retval = 0;
	int* status = wargs->status->status;
	pthread_mutex_lock(&status_mutex);
	//printf("%d - %d\n", *status, ptask_queue->size);
	while(!*status) {
		pthread_mutex_unlock(&status_mutex);
		ptaskq_item* task_item = (ptask_queue->next(ptask_queue, status));
		if(task_item != NULL && task_item->task) {
			ptask_t* task = task_item->task;
			void* routine_res = exec(task);
			if(routine_res) {
				exit_queue->insert(exit_queue, routine_res);
			}
		}
		//printf("%d: checking status %d - %d\n", id, *status, ptask_queue->size);
		pthread_mutex_lock(&status_mutex);
	}
	pthread_mutex_unlock(&status_mutex);
	void* retval_v = malloc(sizeof(int));
	memcpy(retval_v, &retval, sizeof(int));
	wargs->status->exitworker[id] = 1;
	//printf("%d exit\n", id);
	free(wargs);
	pthread_exit(retval_v);
}

void destroy_pthreadpool(pthreadpool_t* tp) {
	tp->status->ptask_q->destroy(tp->status->ptask_q);
	tp->status->exits->destroy(tp->status->exits);
	free(tp->workers);
	free(tp->status->exitworker);
	free(tp->status);
	free(tp);
}
