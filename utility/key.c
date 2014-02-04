char *keytable[256];
unsigned long keytable_tmp[256];

#include "key.h"
#include "keys.h" // has KEYS_BUNDLE
#include "j0g.h"

void key_init()
{
  memset(keytable,0,sizeof(keytable));
  memset(keytable_tmp,0,sizeof(keytable_tmp));
  key_map("OVERFLOW",0); // becomes 0, error
  key_load(KEYS_BUNDLE, 0);
}

int key_map(char *key, int tmp)
{
  int i, old = -1;
  if(strlen(key) > 8) return 0;
  for(i = 0; keytable[i] && i < 256; i++)
  {
    if(keytable_tmp[i] && (old == -1 || keytable_tmp[i] < keytable_tmp[old])) old = i; // track oldest tmp
    if(strcmp(keytable[i],key) != 0) continue;
    if(!tmp) keytable_tmp[i] = 0; // always make sticky if not tmp
    return i;
  }
  // replace a tmp one
  if(i == 256)
  {
    if(old == -1) return 0; // full!
    key_free(old);
    i = old;
  }
  keytable[i] = strdup(key);
  if(tmp) keytable_tmp[i] = millis();
  return i;
}

char *key_get(int i)
{
  if(i < 0 || i > 255) return 0;
  return keytable[i];
}

void key_free(int i)
{
  if(i < 1 || i > 255) return;
  free(keytable[i]);
  keytable[i] = 0;
}

// loads json array of strings, will optionally write new json array of ints into out
void key_load(char *array, char *out)
{
  unsigned int *index, i;
  if(!array || !*array) return;
  index = malloc(strlen(array));
  j0g(array,index,strlen(array));
  if(out) sprintf(out,"[");
  for(i=0;index[i];i+=2)
  {
    int k = key_map(j0g_safe(i,array,index),0);
    if(k && out) sprintf(out+strlen(out),"%d,",k);
  }
  // remove comma if there was any entries
  if(out) sprintf(out+(strlen(out)-(i>0?1:0)),"]");
}
