#ifndef __HTABLE__
#define __HTABLE__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

#define LOAD_FACTOR 0.75f
#define DEFAULT_SIZE 4999

typedef struct _htable_entry {
	void* key;
	void* data;
	struct _htable_entry* prec;
	struct _htable_entry* next;
} htable_entry;

typedef struct _htable_entry_l {
	htable_entry* head;
	htable_entry* tail;
	int size;
} htable_entry_l;

typedef struct _htable {
	long nentries; 							// number of item in hash table
	long nkeys; 							// number of keys mapped 
	float load_factor;						// maximum value for load factor = nentries / size
	long size;								// number of keys alloc'd
	htable_entry_l** keys;					// keys array with length = size && valid items = nkeys
	unsigned int (*htable_hashf)(void*);	// hashing function
    int (*key_compare)(void*, void*);  		// key compare function
    pthread_mutex_t mutex;
    void (*put) (struct _htable*, void*, void*, size_t);
    void* (*get) (struct _htable*, void*, size_t);
    void (*destroy) (struct _htable*);
    void (*destroy_key) (void*);
    void (*destroy_val) (void*);
} htable;


#define INT_BITS     	( sizeof(int) * CHAR_BIT )
#define THREE_QUARTERS  ((int) ((INT_BITS * 3) / 4))
#define ONE_EIGTH      ((int) (INT_BITS / 8))
#define HIGH_BITS       ( ~((unsigned int)(~0) >> ONE_EIGTH ))


htable* new_htable(/* default load factor is LOAD_FACTOR */void (*destroy_key) (void*), \
					void (*destroy_val) (void*));

htable* new_htable_lf(float load_factor, void (*destroy_key) (void*), \
    void (*destroy_val) (void*));

void put(htable* ht, void* key, void* value, size_t size);

void* get(htable* ht, void* key, size_t size);

#endif
