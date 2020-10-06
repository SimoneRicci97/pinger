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
	void (*destroy) (struct _ping_time_chunk*);
	struct _ping_time_chunk* next;
	struct _ping_time_chunk* prec;
} ping_time_chunk;


typedef struct _chunk_list {
	ping_time_chunk* head;
	ping_time_chunk* tail;
	int size;
	ping_stats* global_stats;
	void (*add) (struct _chunk_list*, ping_time_chunk*);
	void (*destroy) (struct _chunk_list*);
} chunk_list;

ping_time_chunk* new_ping_chunk(long chunk_size);

chunk_list* new_chunk_list();