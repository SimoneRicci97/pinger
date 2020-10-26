#include "utlist.h"
#include <stdlib.h>
#include <stdio.h>


utlist_t* new_utlist(void (*item_destroy) (void*), int (*item_compare) (void*, void*)) {
	utlist_t* l = malloc(sizeof(utlist_t));
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;
	l->item_destroy = item_destroy;
	l->item_compare = item_compare;
	return l;
}

void utlist_append(utlist_t* l, void* item) {
	utnode_t* node = malloc(sizeof(utnode_t));
	node->next = NULL;
	node->prec = NULL;
	node->item = item;

	if(l->head == NULL) {
		l->head = node;
	}
	node->prec = l->tail;
	if(l->tail) l->tail->next = node;
	l->tail = node;
	l->size++;
}

void* utlist_index_get(utlist_t* l, int index) {
	utnode_t* ptr = l->head;
	int i = 0;
	while(ptr != NULL && i < index) {
		ptr = ptr->next;
		i++;
	}
	return ptr->item;
}

void* utlist_item_get(utlist_t* l, void* item) {
	utnode_t* ptr = l->head;
	while(ptr != NULL && l->item_compare(ptr->item, item)) {
		ptr = ptr->next;
	}
	return ptr ? ptr->item : NULL;
}

int utlist_index(utlist_t* l, void* item) {
	utnode_t* ptr = l->head;
	int i = 0;
	while(ptr != NULL && l->item_compare(ptr->item, item)) {
		ptr = ptr->next;
		i++;
	}
	return ptr ? i : -1;
}

void utlist_destroy(utlist_t* l) {
	utnode_t* ptr = l->head;
	while(l->head) {
		l->head = l->head->next;
		if(l->head) l->head->next->prec = NULL;
		ptr->next = NULL;
		l->item_destroy(ptr->item);
		free(ptr);
		ptr = l->head;
	}
	free(l);
}