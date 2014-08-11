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
#include "modules/Module.h"

using namespace pinoccio;

Module* ModuleHandler::_modules = NULL;

void ModuleHandler::setup() {
}

void ModuleHandler::loop() {
  // TODO: Make sure that loaded modules are always at the front of the
  //       list, so we don't have to iterate all non-loaded modules as
  //       well?
  Module *module = modules();
  while (module) {
    if (module->loaded())
      module->loop();
    module = module->next();
  }
}

void ModuleHandler::list() {
  const Module *module = modules();
  while (module) {
    speol(module->name());
    module = module->next();
  }
}

bool ModuleHandler::loaded(const char *name) {
  const Module *module = modules();
  while (module) {
    if (strcmp_P(name, (const char*)module->name()) == 0)
      return module->loaded();

    module = module->next();
  }
  return false;
}

Module *ModuleHandler::load(const char *name) {
  Module *module = modules();
  while (module) {
    if (strcmp_P(name, (const char*)module->name()) == 0) {
      if (module->loaded())
        return module;

      if (module->load()) {
        module->_loaded = true;
        return module;
      }

      return NULL;
    }
    module = module->next();
  }

  speol("No such module");
  return NULL;
}

void ModuleHandler::loaded() {
  const Module *module = modules();
  while (module) {
    if (module->loaded())
      speol(module->name());
    module = module->next();
  }
}
