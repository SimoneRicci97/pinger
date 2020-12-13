#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <pthreadpool.h>


#define TEST_SIZE 10000000
#define SMALL_CHUNKS 1 / 100
#define BIG_CHUNKS 1 / 10

typedef struct timeval mtime;

typedef struct _chunk {
	int* a;
	int begin;
	int end;
} chunk_t;

void* sum10_routine(void* args);
ulong test_1thread(int* a);
ulong test_nthread(int* a, int nthreads, int chunksize);
void nthreads_report(int* a, int nthreads, int chunksize);
ulong gettimeofdaymicros();

int main(int argc, char const *argv[])
{
	int* a = malloc(TEST_SIZE * sizeof(int));
	ulong t0, t1;
	ulong sum;
	srand((unsigned int) time(NULL));
	for(int i=0; i<TEST_SIZE; i++) {
		a[i] = ((int) rand()) % 20;
	}
	fprintf(stderr, "Array initialized\n");

	t0 = gettimeofdaymicros();
	sum = test_1thread(a);
	t1 = gettimeofdaymicros();
	fprintf(stderr, "Single thread\n");
	fprintf(stderr, "\tSum: %ld\n", sum);
	fprintf(stderr, "\tExecution time: %.2f micros\n", (t1 - t0) / 1000.f);

	printf("\n");
	nthreads_report(a, 2, TEST_SIZE * SMALL_CHUNKS);
	printf("\n");
	nthreads_report(a, 2, TEST_SIZE * BIG_CHUNKS);
	printf("\n");
	nthreads_report(a, 4, TEST_SIZE * SMALL_CHUNKS);
	printf("\n");
	nthreads_report(a, 4, TEST_SIZE * BIG_CHUNKS);
	printf("\n");
	nthreads_report(a, 8, TEST_SIZE * SMALL_CHUNKS);
	printf("\n");
	nthreads_report(a, 8, TEST_SIZE * BIG_CHUNKS);

	free(a);
}

void nthreads_report(int* a, int nthreads, int chunksize) {
	ulong t0, t1;
	ulong sum;
	t0 = gettimeofdaymicros();
	sum = test_nthread(a, nthreads, chunksize);
	t1 = gettimeofdaymicros();
	fprintf(stderr, "%d threads\n", nthreads);
	fprintf(stderr, "Chunksize: %d\n", chunksize);
	fprintf(stderr, "\tSum: %ld\n", sum);
	fprintf(stderr, "\tExecution time: %.2f micros\n", (t1 - t0) / 1000.f);
}

ulong test_1thread(int* a) {
	ulong sum = 0;
	for(int i=0; i<TEST_SIZE; i++) {
		sum += a[i];
	}
	return sum;
}

ulong test_nthread(int* a, int nthreads, int chunksize) {
	int TERM = 0;
	printf("Test with %d threads and chunksize = %d\n", nthreads, chunksize);
	printf("N tasks: %d\n", TEST_SIZE/chunksize);
	pthreadpool_t* tp = new_threadpool(nthreads, &TERM, 0);
	for(int i=0; i<TEST_SIZE; i+=chunksize) {
		chunk_t* args = malloc(sizeof(chunk_t));
		args->a = a;
		args->begin = i;
		args->end = i + chunksize;
		ptp_add_task(tp, sum10_routine, args);
	}
	ptp_start(tp);
	int freed=0;
	ulong totsum = 0;
	int returned = 0;
	while(returned<TEST_SIZE/chunksize) {
		exit_item_t* retval = pretval_get(tp->status->exits, tp->status->status);
		if(retval) {
			ulong* chunksum = malloc(sizeof(ulong));
			memcpy(chunksum, retval->exit_status, sizeof(ulong));
			totsum += *chunksum;
			returned ++;
			free(chunksum);
			free(retval->exit_status);
			free(retval);
			freed++;
		}
	}
//	printf("freed %d els\n", freed);
	ptp_stop(tp);
	destroy_pthreadpool(tp);
	return totsum;
}

void* sum10_routine(void* args) {
	chunk_t* chunk = malloc(sizeof(chunk_t));
	memcpy(chunk, args, sizeof(chunk_t));
	ulong sum = 0;
	for(int i=chunk->begin; i<chunk->end; i++) {
		sum += chunk->a[i];
	} 
	void* ret = malloc(sizeof(ulong));
	memcpy(ret, &sum, sizeof(ulong));
	free(chunk);
	return ret;
}

ulong gettimeofdaymicros() {
	mtime t;
	gettimeofday(&t, NULL);
	ulong s1 = t.tv_sec * 1000 * 1000;
	ulong s2 = t.tv_usec;
	return s1 + s2;
}
