#ifndef __PINGER_SIGNAL__
#define __PINGER_SIGNAL__ 

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <alarm_handler.h>

typedef struct _sigs_handling
{
	sigset_t sigset;
	struct sigaction sigs_handlers;
}sigs_handling;

typedef struct _handler_args{
	sigs_handling* sighandl;
	int* termflag;
	alarmh_t* alarm_handler;
} handler_args_t;

void* handling_r(void* sigmask_r);

void maskall(sigset_t sigset, struct sigaction* s);

#endif
