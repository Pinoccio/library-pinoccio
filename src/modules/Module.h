/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_MODULE_H_
#define LIB_PINOCCIO_MODULE_H_

#include <Arduino.h>
#include "ModuleHandler.h"

namespace pinoccio {
  class Module {
    public:
      Module() {
        // Prepend ourselves to the list of modules
        this->_next = ModuleHandler::_modules;
        ModuleHandler::_modules = this;
      }

      /**
       * Called when the module is enabled
       */
      virtual bool enable() = 0;

      /**
       * Called on every loop, while the module is enabled.
       */
      virtual void loop() = 0;

      /**
       * Returns the module name
       */
      virtual const __FlashStringHelper* name() const = 0;

      /**
       * Is this module loaded?
       */
      bool enabled() const { return _enabled; }

      /**
       * Next element in the list of modules
       */
      Module* next() const { return _next; }

    private:
      bool _enabled;
      Module* _next;

      // Let ModuleHandler update _enabled
      friend class ModuleHandler;
  };
} // namespace pinoccio
#endif
