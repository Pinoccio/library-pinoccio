void key_init();
int key_map(char *key, int tmp);
char *key_get(int i);
void key_free(int i);
// loads json array of strings, will optionally write new json array of ints into out
void key_load(char *array, char *out);
