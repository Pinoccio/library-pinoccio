#include "key.h"
#include "keys.h" // has KEYS_BUNDLE
#include "j0g.h"

char *keytable[KEY_MAX];
char keytable_tmp[KEY_MAX];
unsigned long keytable_last = 0;

void key_init()
{
  memset(keytable,0,sizeof(keytable));
  memset(keytable_tmp,0,sizeof(keytable_tmp));
  key_map("OVERFLOW",0); // becomes 0, error
  key_load(KEYS_BUNDLE, 0, 0);
}

// idea was to use now to do time based expirations, but for now just expiring all temps immediately
int key_loop(unsigned long now)
{
  int i;
  if(!keytable_last) return 0;

  // free any tmp keys
  for(i = 0; i < KEY_MAX; i++)
  {
    if(!keytable_tmp[i]) continue;
    key_free(i);
  }
  keytable_last = 0;
  return 1;
}

int key_map(char *key, unsigned long at)
{
  int i;
  if(strlen(key) > KEY_LEN) return 0;
  for(i = 0; keytable[i] && i < KEY_MAX; i++)
  {
    if(strcmp(keytable[i],key) != 0) continue;
    if(!at) keytable_tmp[i] = 0; // always make sticky if not tmp
    return i;
  }
  // full!
  if(i == KEY_MAX) return 0;
  // save new key
  keytable[i] = strdup(key);
  if(at) {
    keytable_last = at;
    keytable_tmp[i] = 1;
  }
  return i;
}

char *key_get(int i)
{
  if(i < 0 || i >= KEY_MAX) return 0;
  return keytable[i];
}

void key_free(int i)
{
  if(i < 1 || i >= KEY_MAX) return;
  free(keytable[i]);
  keytable_tmp[i] = 0;
  keytable[i] = 0;
}

// loads json array of strings, outs is optional
void key_load(char *array, int *outs, unsigned long at)
{
  unsigned int *index, i, oi=1;
  if(!array || !*array) return;
  index = malloc(strlen(array));
  j0g(array,index,strlen(array));
  for(i=0;index[i];i+=2)
  {
    int k = key_map(j0g_safe(i,array,index),at);
    if(outs) outs[oi++] = k;
  }
  if(outs) outs[0] = oi-1;
}
