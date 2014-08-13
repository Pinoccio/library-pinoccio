/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include "Scout.h"
#include "ModuleHandler.h"
#include "Module.h"

using namespace pinoccio;

Module* ModuleHandler::_modules = NULL;

void ModuleHandler::setup() {
}

void ModuleHandler::loop() {
  // TODO: Make sure that enabled modules are always at the front of the
  //       list, so we don't have to iterate all disabled modules as
  //       well?
  Module *module = modules();
  while (module) {
    if (module->enabled())
      module->loop();
    module = module->next();
  }
}

Module *ModuleHandler::enable(const char *name) {
  Module *module = modules();
  while (module) {
    if (strcmp_P(name, (const char*)module->name()) == 0) {
      if (module->enabled())
        return module;

      if (module->enable()) {
        module->_enabled = true;
        return module;
      }

      return NULL;
    }
    module = module->next();
  }

  speol("No such module");
  return NULL;
}
