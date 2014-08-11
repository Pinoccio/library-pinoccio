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


namespace pinoccio {
  class Module;

  class ModuleHandler {
    public:
      ModuleHandler();

      static void setup();
      static void loop();

      static Module *load(const char *name);
      static bool loaded(const char *name);
      static void list();
      static void loaded();

      /**
       * Return the first module in the list of all modules. Subsequent
       * modules can be iterated by calling the next() method on each
       * module.
       */
      static Module* modules() {
        return _modules;
      }

    protected:
      static Module* _modules;

      // Let PinoccioModule update _modules to register itself
      friend class Module;
  };
} // namespace pinoccio

#endif
