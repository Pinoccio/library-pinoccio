#ifndef _key_h_
#define _key_h_

#define KEY_MAX 64
#define KEY_LEN 10

void key_init();
int key_loop(unsigned long now);
// at is current millis, if set will expire it at next key_loop call
int key_map(char *key, unsigned long at);
char *key_get(int i);
void key_free(int i);
// loads json array of strings, will optionally save values into outs (caller provided)
void key_load(char *array, int *outs, unsigned long at);

#endif