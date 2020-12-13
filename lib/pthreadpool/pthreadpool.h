#ifndef __PTHREADPOOL__
#define __PTHREADPOOL__ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "ptaskqueue.h"
#include "pretvalqueue.h"

typedef struct _threadpool_status {
	ptaskq_t* ptask_q;
	exitq_t* exits;
	int* status;
	short* exitworker;
	pthread_mutex_t status_mutex;
} threadpool_status;


typedef struct _pthreadpool {
	pthread_t* workers;
	size_t size;
	threadpool_status* status;
} pthreadpool_t;



pthreadpool_t* new_threadpool(size_t size, int* termflag, short circle);

void destroy_pthreadpool(pthreadpool_t* tp);

void ptp_wait(pthreadpool_t* tp);
void ptp_start(pthreadpool_t* tp);
void ptp_stop(pthreadpool_t* tp);
void ptp_waitstop(pthreadpool_t* tp);
void ptp_add_task(pthreadpool_t* tp, void* (*task) (void*), void* taskarg);

#endif
