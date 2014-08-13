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
unsigned long keytableLast = 0;

void keyInit() {
  memset(keytable, 0, sizeof(keytable));
  memset(keytableTmp, 0, sizeof(keytableTmp));
  keyMap("OVERFLOW", 0); // becomes 0, error
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

int keyMap(const char *key, unsigned long at) {
  int i;

  for (i=0; keytable[i] && i<KEY_MAX; i++) {
    if (strcmp(keytable[i],key) != 0) {
      continue;
    }
    if (!at) {
      keytableTmp[i] = 0; // always make sticky if not tmp
    }
    return i;
  }

  // full!
  if (i == KEY_MAX) {
    return 0;
  }

  // save new key
  keytable[i] = strdup(key);
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
  free(keytable[i]);
  keytableTmp[i] = 0;
  keytable[i] = 0;
}

// loads json array of strings, outs is optional
void keyLoad(const char *array, int *outs, unsigned long at) {
  unsigned short *index;
  unsigned int i;
  unsigned int oi=1;

  if (!array || !*array) {
    return;
  }

  index = (unsigned short*)malloc(strlen(array));
  if (!index) {
    outs[0] = 0;
    return;
  }
  j0g((char*)array, index, strlen(array));

  for (i=0; index[i]; i+=2) {
    int k = keyMap(j0g_safe(i, (char*)array, index), at);
    if (outs) {
      outs[oi++] = k;
    }
  }

  if (outs) {
    outs[0] = oi-1;
  }

  free(index);
}
