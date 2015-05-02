struct entry {
	char *key;
	char *value;
	struct entry *next;
};

char *map_get(char *key);
int map_set(char *key, char *value);
void map_print();

