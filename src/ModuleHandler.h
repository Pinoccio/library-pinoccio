/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_MODULE_HANDLER_H_
#define LIB_PINOCCIO_MODULE_HANDLER_H_

#include "util/LinkedList.h"
#include <modules/PinoccioModule.h>

class PinoccioModuleHandler {
  public:
    static void add(PinoccioModule* module);

    static void setup();
    static void loop();
};

typedef PinoccioModuleHandler ModuleHandler;
#endif