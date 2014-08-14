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


namespace pinoccio {
  class Module;

  class ModuleHandler {
    public:
      ModuleHandler();

      static void setup();
      static void loop();

      /**
       * Enable the module with the given name.
       *
       * Returns the module if it was enabled or already enabled,
       * returns NULL when the module was not found or enabling failed.
       */
      static Module *enable(const char *name);

      /**
       * Enables the given module.
       */
      static Module *enable(Module *module);

      /**
       * Finds the module with the given name, or NULL.
       */
      static Module *find(const char *name);

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
