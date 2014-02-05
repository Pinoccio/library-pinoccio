#ifndef _key_h_
#define _key_h_

#define KEY_MAX 64

void key_init();
void key_loop();
int key_map(char *key, int tmp);
char *key_get(int i);
void key_free(int i);
// loads json array of strings, will optionally write new json array of ints into out
void key_load(char *array, char *out);

#endif