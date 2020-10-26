#ifndef __UTLIST__
#define __UTLIST__ 

typedef struct _utnode {
	void* item;
	struct _utnode* next;
	struct _utnode* prec;
} utnode_t;

typedef struct _utlist {
	utnode_t* head;
	utnode_t* tail;
	unsigned int size;
	void (*item_destroy) (void*);
	int (*item_compare) (void*, void*);
} utlist_t;


utlist_t* new_utlist(void (*item_destroy) (void*), int (*item_compare) (void*, void*));

void utlist_append(utlist_t* l, void* item);

void* utlist_index_get(utlist_t* l, int index);

void* utlist_item_get(utlist_t* l, void* item);

int utlist_index(utlist_t* l, void* item);

void utlist_destroy(utlist_t* l);


#endif