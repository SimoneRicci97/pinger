#ifndef __ALARM_HANDLER__
#define __ALARM_HANDLER__

typedef struct _alarm_h {
	void (*alarm_r) (void*);
	void* arg_r;
	int alrm_sec;
	struct _alarm_h** eachn_r;
} alarmh_t;

#endif
