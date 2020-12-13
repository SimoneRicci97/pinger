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

void* ptp_worker(void* args_queue);

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

	pthread_mutex_init(&tp->status->status_mutex, NULL);
	return tp;
}

void ptp_add_task(pthreadpool_t* tp, void* (*task) (void*), void* taskarg) {
	ptask_t* ptask = malloc(sizeof(ptask_t));
	ptask->t_arg = malloc(sizeof(ptaskarg_t));
	ptask->t_arg->t_arg = taskarg;
	ptask->t_arg->status = tp->status->status;
	ptask->routine = task;
	ptaskq_add(tp->status->ptask_q, ptask);
}

void ptp_stop(pthreadpool_t* tp) {
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


void ptp_waitstop(pthreadpool_t* tp) {
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


void ptp_wait(pthreadpool_t* tp) {
	pthread_mutex_lock(&tp->status->ptask_q->mutex);
	while(tp->status->ptask_q->size > 0) {
		pthread_cond_wait(&tp->status->ptask_q->nempty_semaphore, &tp->status->ptask_q->mutex);
	}
	pthread_mutex_unlock(&tp->status->ptask_q->mutex);
}

void ptp_start(pthreadpool_t* tp) {
	for(int i=0; i<tp->size; i++) {
		wargs_t* wargs = malloc(sizeof(wargs_t));
		wargs->wid = i;
		wargs->status = tp->status;
		int tid = pthread_create(&tp->workers[i], NULL, ptp_worker, wargs);
		if(tid) {
			perror("Creating workers");
			exit(1);
		}
	}
}


void* ptp_worker(void* __args) {
	wargs_t* wargs = (wargs_t*) __args;
	ptaskq_t* ptask_queue = wargs->status->ptask_q;
	exitq_t* exit_queue = wargs->status->exits;
	int id = wargs->wid;
	pthread_mutex_t status_mutex = wargs->status->status_mutex;
	int retval = 0;
	int* status = wargs->status->status;
	pthread_mutex_lock(&status_mutex);
	while(!*status) {
		pthread_mutex_unlock(&status_mutex);
		ptaskq_item* task_item = ptaskq_next(ptask_queue, status);
		if(task_item != NULL && task_item->task) {
			ptask_t* task = task_item->task;
			void* routine_res = exec(task);
			if(routine_res) {
				pretval_insert(exit_queue, routine_res);
			}
		}
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
	ptaskq_destroy(tp->status->ptask_q);
	pretval_destroy(tp->status->exits);
	free(tp->workers);
	free(tp->status->exitworker);
	free(tp->status);
	free(tp);
}
