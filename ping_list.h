typedef struct _ping_time {
	float interval;
	struct _ping_time* next;
	struct _ping_time* prec;
} ping_time;

typedef struct _ping_time_list {
	ping_time* head;
	ping_time* tail;
	long size;
	float avg;
	void (*add) (struct _ping_time_list*, float);
} ping_time_list;

ping_time_list* new_ping_list();

void _add(ping_time_list* l, float interval);