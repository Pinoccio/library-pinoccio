#ifndef _key_h_
#define _key_h_

#define KEY_MAX 64
#define KEY_LEN 10

void keyInit();

int keyLoop(unsigned long now);

// at is current millis, if set will expire it at next key_loop call
int keyMap(const char *key, unsigned long at);

const char* keyGet(int i);

void keyFree(int i);

// loads json array of strings, will optionally save values into outs (caller provided)
void keyLoad(const char *array, int *outs, unsigned long at);

#endif
