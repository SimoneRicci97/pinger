#ifndef __PINGER_REPORT__
#define __PINGER_REPORT__
#include <htable.h>

#define PINGER_OUT_PREFIX ".pinger.log"

typedef struct _report_args {
	htable* hosts;
	char* reportout;
} report_args_t;

void do_report(void* report_args);

#endif
