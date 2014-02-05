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
  key_load(KEYS_BUNDLE, 0);
}

void key_loop()
{
  int i;
  if(!keytable_last) return;
  if(millis() - keytable_last < 10*1000) return;

  // free any tmp keys
  for(i = 0; keytable[i] && i < KEY_MAX; i++)
  {
    if(!keytable_tmp[i]) continue;
    key_free(i);
  }  
}

int key_map(char *key, int tmp)
{
  int i;
  if(strlen(key) > 8) return 0;
  for(i = 0; keytable[i] && i < KEY_MAX; i++)
  {
    if(strcmp(keytable[i],key) != 0) continue;
    if(!tmp) keytable_tmp[i] = 0; // always make sticky if not tmp
    return i;
  }
  // full!
  if(i == KEY_MAX) return 0;
  // save new key
  keytable[i] = strdup(key);
  if(tmp) {
    keytable_last = millis();    
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
