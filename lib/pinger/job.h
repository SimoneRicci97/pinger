#ifndef __PINGER_JOB__
#define __PINGER_JOB__

#include "htable.h"

typedef struct _pinger_thread_args {
	char* host;
	char* n_ping;
	int runs;
	htable* hosts_data;
} pinger_thread_args;

void* t_doping(void* args);


#endif
