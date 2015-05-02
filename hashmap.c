#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "hashmap.h"

struct entry map;

char *map_get(char *key) {
	struct entry *iterator = map.next;
	while(iterator != NULL) {
		if (strcmp(iterator->key, key) == 0) {
			return iterator->value;
		}
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
			free(current->value);
			current->value = malloc(strlen(value));
			strcpy(current->value, value);
			return 1;
		}
		iterator = iterator->next;
	}
	newEntry = malloc(sizeof(struct entry));
	iterator->next = newEntry;
	newEntry->key = malloc(strlen(key));
	strcpy(newEntry->key, key);
	newEntry->value = malloc(strlen(value));
	strcpy(newEntry->value, value);
	newEntry->next = NULL;
	return 1;
}

void map_print() {
	struct entry *iterator = map.next;
	while(iterator != NULL) {
		printf("key %s, value %s\n",iterator->key, iterator->value);
		iterator = iterator->next;
	}
}
