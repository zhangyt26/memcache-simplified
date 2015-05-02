#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "hashmap.h"

struct entry map;

void map_init() {
	map.rw_lock = malloc(sizeof(pthread_rwlock_t));
	pthread_rwlock_init(map.rw_lock, NULL);
}

char *map_get(char *key) {
	struct entry *iterator = map.next;
	while(iterator != NULL) {
		if (pthread_rwlock_rdlock(iterator->rw_lock) == 0) {
			if (strcmp(iterator->key, key) == 0) {
				pthread_rwlock_unlock(iterator->rw_lock);
				return iterator->value;
			}
		}
		pthread_rwlock_unlock(iterator->rw_lock);
		iterator = iterator->next;

	}
	return NULL;
}


int map_set(char *key, char *value) {
	struct entry *iterator = &map;
	struct entry *newEntry;

	while(iterator->next != NULL) {
		struct entry *current = iterator->next;
		if (strcmp(current->key, key) == 0) {
			printf("modifying an existing value\n");
			if (pthread_rwlock_wrlock(current->rw_lock) == 0) {
				free(current->value);
				current->value = malloc(strlen(value));
				strcpy(current->value, value);
			}
			pthread_rwlock_unlock(current->rw_lock);
			return 1;
		}
		iterator = iterator->next;
	}
	printf("create a new value\n");
	if (pthread_rwlock_wrlock(iterator->rw_lock) == 0) { // lock the last element
		newEntry = malloc(sizeof(struct entry));
		iterator->next = newEntry;
		newEntry->key = malloc(strlen(key));
		strcpy(newEntry->key, key);
		newEntry->value = malloc(strlen(value));
		strcpy(newEntry->value, value);
		newEntry->next = NULL;
		newEntry->rw_lock = malloc(sizeof(pthread_rwlock_t));
		pthread_rwlock_init(newEntry->rw_lock, NULL);
	}
	pthread_rwlock_unlock(iterator->rw_lock);
	return 1;
}

void map_print() {
	struct entry *iterator = map.next;
	while(iterator != NULL) {
		printf("key %s, value %s\n",iterator->key, iterator->value);
		iterator = iterator->next;
	}
}
