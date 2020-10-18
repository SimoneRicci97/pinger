#ifndef __PTASK__
#define __PTASK__ value

typedef struct _ptaskarg {
	int* status;
	void* t_arg;
} ptaskarg_t;

typedef struct _ptask {
	void* (*routine) (void*);
	ptaskarg_t* t_arg;
} ptask_t;

void* exec(ptask_t* task);

void ptask_destroy(ptask_t* ptask);

#endif
