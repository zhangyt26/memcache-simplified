struct entry {
	char *key;
	char *value;
	pthread_rwlock_t *rw_lock;
	struct entry *next;
};

void map_init();
char *map_get(char *key);
int map_set(char *key, char *value);
void map_print();

