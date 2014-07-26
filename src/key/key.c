/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include "key.h"
#include "keys.h" // has KEYS_BUNDLE
#include "j0g.h"
#include <string.h>
#include <stdlib.h>

char *keytable[KEY_MAX];
char keytableTmp[KEY_MAX];
unsigned char keyRef[KEY_MAX];
unsigned long keytableLast = 0;

void keyInit() {
  memset(keytable, 0, sizeof(keytable));
  memset(keytableTmp, 0, sizeof(keytableTmp));
  memset(keyRef, 0, sizeof(keyRef));
  keyMapRO("OVERFLOW", 0); // becomes 0, error
  keyLoad(KEYS_BUNDLE, 0, 0);
}

// idea was to use now to do time based expirations, but for now just expiring all temps immediately
int keyLoop(unsigned long now) {
  int i;
  if (!keytableLast) {
    return 0;
  }

  // free any tmp keys
  for (i=0; i<KEY_MAX; i++) {
    if (!keytableTmp[i]) {
      continue;
    }
    keyFree(i);
  }

  keytableLast = 0;
  return 1;
}

int keyMapRO(const char *key, unsigned long at) {
  int i;
  if (strlen(key) > KEY_LEN) {
    return 0;
  }
  keyRef[i] = 0xFF;
  return keyMap(key, at);
}

int keyMap(const char *key, unsigned long at) {
  int i;

  for (i=0; keytable[i] && i<KEY_MAX; i++) {
    if (strcmp(keytable[i],key) != 0) {
      continue;
    }
    if (!at) {
      keytableTmp[i] = 0; // always make sticky if not tmp
    }
    // 0xFF mean "never free"
    if(keyRef[i] != 0xFF)
    {
      // reusing the same key, so just inc the reference count
      keyRef[i]++;
    }
    return i;
  }

  // full!
  if (i == KEY_MAX) {
    return 0;
  }

  // save new key
  keytable[i] = strdup(key);
  keyRef[i] = 1;    // initialize the keyRef
  if (at) {
    keytableLast = at;
    keytableTmp[i] = 1;
  }
  return i;
}

const char *keyGet(int i) {
  if (i < 0 || i >= KEY_MAX) {
    return 0;
  }
  return keytable[i];
}

void keyFree(int i) {
  if (i < 1 || i >= KEY_MAX) {
    return;
  }
  // don't free if ref is 0xFF
  if(keyRef == 0xFF)
  {
    return;
  }
  if(keyRef[i] == 1 || keyRef[i] == 0)
  {
    free(keytable[i]);
    keytableTmp[i] = 0;
    keytable[i] = 0;
    keyRef[i] = 0;
  }
  else
  {
    keyRef[i]--;
  }
}

// loads json array of strings, outs is optional
void keyLoad(const char *array, int *outs, unsigned long at) {
  unsigned int *index;
  unsigned int i;
  unsigned int oi=1;

  if (!array || !*array) {
    return;
  }

  index = malloc(strlen(array));
  if (!index) {
    outs[0] = 0;
    return;
  }
  j0g(array, index, strlen(array));

  for (i=0; index[i]; i+=2) {
    int k = keyMapRO(j0g_safe(i, array, index), at);
    if (outs) {
      outs[oi++] = k;
    }
  }

  if (outs) {
    outs[0] = oi-1;
  }

  free(index);
}
