typedef struct _ping_time {
	float interval;
	struct _ping_time* next;
	struct _ping_time* prec;
} ping_time;

typedef struct _ping_stats {
	float avg;
	float min;
	float max;
	float stdev;
	int loss;
} ping_stats;

typedef struct _ping_time_chunk {
	ping_time* values;
	long index;
	long size;
	ping_stats* chunk_stats;
	void (*add) (struct _ping_time_chunk*, float);
} ping_time_chunk;


ping_time_chunk* new_ping_chunk(long chunk_size);

void _add(ping_time_chunk* l, float interval);