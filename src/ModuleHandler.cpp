/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Scout.h>
#include <ModuleHandler.h>

LinkedList<PinoccioModule*> ModuleHandler::loadedModules;
const PinoccioModuleInfoBase* ModuleHandler::moduleInfo = NULL;

void ModuleHandler::setup() {
}

void ModuleHandler::loop() {
  for (uint8_t i=0; i<loadedModules.size(); i++) {
    (loadedModules.get(i))->loop();
  }
}

void ModuleHandler::list() {
  const PinoccioModuleInfoBase *info = moduleInfo;
  while (info) {
    speol(info->name);
    info = info->next;
  }
}

bool ModuleHandler::loaded(char *name) {
  for (uint8_t i=0; i<loadedModules.size(); i++) {
    if(strcmp(loadedModules.get(i)->name(),name) == 0) return true;
  }
  
  return false;
}

PinoccioModule *ModuleHandler::load(char *name) {
  for (uint8_t i=0; i<loadedModules.size(); i++) {
    PinoccioModule *module = loadedModules.get(i);
    if(strcmp(module->name(),name) == 0) return module;
  }

  const PinoccioModuleInfoBase *info = moduleInfo;
  while (info) {
    if (strcmp(info->name, name) == 0) {
      PinoccioModule *module = info->load();
      loadedModules.add(module);
      module->setup();
      return module;
    }
    info = info->next;
  }

  return NULL;
}

void ModuleHandler::loaded() {
  for (uint8_t i=0; i<loadedModules.size(); i++) {
    speol((loadedModules.get(i))->name());
  }
}
