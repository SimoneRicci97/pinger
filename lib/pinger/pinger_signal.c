#include "pinger_signal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <alarm_handler.h>
#include <uttime.h>

void* handling_r(void* sigmask_r) {
	handler_args_t* handler_arg= ((handler_args_t*) sigmask_r);
	int* termflag = handler_arg->termflag;
	sigset_t sigset = handler_arg->sighandl->sigset;
	int lastsig;
	alarmh_t* alrm_cb = handler_arg->alarm_handler;
	alarmh_t** routines = alrm_cb->eachn_r;
	int alrm_cnt = 0;
	while(!*termflag) {
		sigwait(&sigset, &lastsig);
		switch(lastsig) {
			case SIGINT:
			case SIGKILL:
			case SIGQUIT:
			case SIGTERM:
			case SIGSEGV:
			case SIGSTOP: {
				*termflag = 1;
			} break;
			case SIGUSR1:
			case SIGALRM: {
				alrm_cb->alarm_r(alrm_cb->arg_r);
				alrm_cnt++;
				int i = 0;
				while(routines[i] != NULL) {
					//printf("%d) %d %% %d = %d\n", i, alrm_cnt, routines[i]->alrm_sec, alrm_cnt % routines[i]->alrm_sec);
					if(alrm_cnt % routines[i]->alrm_sec == 0) {
						routines[i]->alarm_r(routines[i]->arg_r);
					}
					i++;
				}
				alarm(alrm_cb->alrm_sec);
			} break;
		}
	}
	return NULL;
}


void maskall(sigset_t sigset, struct sigaction* s) {
	sigfillset(&sigset);
	pthread_sigmask(SIG_SETMASK, &sigset, NULL);
	memset(s, 0, sizeof(*s));
}
