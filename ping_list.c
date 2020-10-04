#include <stdio.h>
#include <stdlib.h>
#include "ping_list.h"

ping_time_list* new_ping_list() {
	ping_time_list* l = malloc(sizeof(ping_time_list));
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;
	l->add = _add;
	return l;
}

void _add(ping_time_list* l, float interval) {
	ping_time* item = malloc(sizeof(ping_time));
	item->next = NULL;
	item->prec = l->tail;
	item->interval = interval;
	if(l->tail != NULL) l->tail->next = item;
	l->tail = item;
	if(l->head == NULL) l->head = item;
	l->size++;
	if(l->size == 1) {
		l->avg = interval;
	} else {
		l->avg = ((l->avg * (l->size - 1)) + interval) / l->size;
	}
}

int main4test(int argc, char const *argv[])
{
	ping_time_list* l = new_ping_list();
	float f = 1.02;
	for(int i=0; i<10; i++) {
		l->add(l, f);
		f = f + 1.f;
	}
	ping_time* ptr = l->head;
	int i = 0;
	while(ptr != NULL) {
		printf("%d: %.2f\n", i, ptr->interval);
		ptr = ptr->next;
		i++;
	}
	printf("printed: %d\nsize: %ld\n", i, l->size);
	return 0;
}