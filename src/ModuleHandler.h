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
#include <modules/Modules.h>

class ModuleHandler {
  public:
    ModuleHandler();
    static void add(PinoccioModule* module);

    static void setup();
    static void loop();
    
    static PinoccioModule *load(char *name);
    static bool loaded(char *name);
    static void list();
    static void loaded();
};

#endif