#include <stdio.h>
#include <stdlib.h>
#include "ping_list.h"

ping_time_chunk* new_ping_chunk(long chunk_size) {
	ping_time_chunk* l = malloc(sizeof(ping_time_chunk));
	l->values = malloc(sizeof(ping_time) * chunk_size);
	l->size = 0;
	l->index = 0;
	l->add = _add;
	return l;
}

void _add(ping_time_chunk* l, float interval) {
	ping_time* item = malloc(sizeof(ping_time));
	item->next = NULL;
	item->prec = NULL;
	item->interval = interval;
	l->values[l->index].interval = interval;
	l->index++;
	l->size++;
	l->chunk_stats = NULL;
}

// int main4test(int argc, char const *argv[])
// {
// 	ping_time_chunk* l = new_ping_chunk(10);
// 	float f = 1.02;
// 	for(int i=0; i<10; i++) {
// 		l->add(l, f);
// 		f = f + 1.f;
// 	}
// 	int i = 0;
// 	for(int i=0; i<l->size; i++) {
// 		ping_time ptr = l->values[i];
// 		printf("%d: %.2f\n", i, ptr.interval);
// 		i++;
// 	}
// 	printf("printed: %d\nsize: %u\n", i, l->size);
// 	return 0;
// }