#include <stdlib.h>
#include <stdio.h>
#include "ptask.h"


void* exec(ptask_t* task) {
	return task->routine(task->t_arg->t_arg);
}


void ptask_destroy(ptask_t* ptask) {
	free(ptask->t_arg->t_arg);
	free(ptask->t_arg);
	free(ptask);
}
