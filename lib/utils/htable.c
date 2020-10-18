#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "htable.h"
#include <limits.h>

htable_entry* __get_by_key(htable* ht, htable_entry_l* hashed, void* key);
long __insert_or_replace(htable* ht, htable_entry_l* hashed, htable_entry* entry);
void _htabledestroy(htable* ht);
void _put(htable* ht, void* key, void* value, size_t size);
void* _get(htable* ht, void* key, size_t size);

unsigned int hash_function(void* key) {
	char *datum = (char *)key;
    unsigned int hash_value, i;

    if(!datum) return 0;

    for (hash_value = 0; *datum; ++datum) {
        hash_value = (hash_value << ONE_EIGTH) + *datum;
        if ((i = hash_value & HIGH_BITS) != 0)
            hash_value = (hash_value ^ (i >> THREE_QUARTERS)) & ~HIGH_BITS;
    }
    return (hash_value) % 4999;
}

int keys_compare(void* a, void* b) {
    return (strcmp( (char*)a, (char*)b ) == 0);
}

htable* new_htable(void (*destroy_key) (void*), void (*destroy_val) (void*)) {
	return new_htable_lf(LOAD_FACTOR, destroy_key, destroy_val);
}

htable* new_htable_lf(float load_factor, void (*destroy_key) (void*), \
	void (*destroy_val) (void*)) {
	
	htable* ht = malloc(sizeof(htable));
	ht->nentries = 0L;
	ht->nkeys = 0L;
	ht->load_factor = load_factor;
	ht->size = DEFAULT_SIZE;
	ht->keys = malloc(ht->size * sizeof(htable_entry*));
	for(long i=0; i<ht->size; i++) {
		ht->keys[i] = NULL;
	}
	ht->htable_hashf = hash_function;
	ht->key_compare = keys_compare;
	ht->destroy = _htabledestroy;
	ht->destroy_key = destroy_key;
	ht->destroy_val = destroy_val;
	ht->put = _put;
	ht->get = _get;
	return ht;
}

void _htabledestroy(htable* ht) {
	for(long i=0; i<ht->size; i++) {
		if(ht->keys[i] != NULL) {
			htable_entry* ptr = ht->keys[i]->head;
			while(ptr != NULL) {
				ht->keys[i]->head = ht->keys[i]->head->next;
				//ht->keys[i]->head->prec = NULL;
				ptr->next = NULL;
				ht->destroy_key(ptr->key);
				ht->destroy_key(ptr->data);
				free(ptr);
				ptr = ht->keys[i]->head;
			}
			free(ht->keys[i]);
		}
	}
	free(ht->keys);
	free(ht);
}

void _put(htable* ht, void* key, void* value, size_t size) {
	htable_entry* entry = malloc(sizeof(htable_entry));
	entry->key = key;
	entry->data = value;
	entry->next = NULL;
	entry->prec = NULL;

	unsigned int hash_code = ht->htable_hashf(key);
	
	if(ht->keys[hash_code] == NULL) {
		ht->keys[hash_code] = malloc(sizeof(htable_entry_l));
		ht->keys[hash_code]->head = entry;
		ht->keys[hash_code]->tail = entry;
		ht->keys[hash_code]->size = 1;
		ht->nkeys++;
		ht->nentries++;
	} else {
		ht->nentries += __insert_or_replace(ht, ht->keys[hash_code], entry);
	}

}


void* _get(htable* ht, void* key, size_t size) {
	unsigned int hash_code = ht->htable_hashf(key);
	htable_entry_l* ptr = ht->keys[hash_code];

	htable_entry* entry = __get_by_key(ht, ptr, key);
	if(entry != NULL) return entry->data;
	return NULL;
}


htable_entry* __get_by_key(htable* ht, htable_entry_l* hashed, void* key) {
	htable_entry* ptr = hashed->head;
	while(ptr != NULL) {
		if(ht->key_compare(ptr->key, key)) {
			return ptr;
		}
		ptr = ptr->next;
	}
	return NULL;
}


long __insert_or_replace(htable* ht, htable_entry_l* hashed, htable_entry* entry) {
	htable_entry* ptr = __get_by_key(ht, hashed, entry->key);
	if(ptr == NULL) {
		hashed->tail->next = entry;
		entry->prec = hashed->tail;
		hashed->tail = entry;
		hashed->size++;
		return 1;
	} else {
		ptr->data = entry->data;
		return 0;
	}
}



