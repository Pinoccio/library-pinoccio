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

LinkedList<PinoccioModule*> moduleList = LinkedList<PinoccioModule*>();

void ModuleHandler::setup() {
}

void ModuleHandler::loop() {
  for (uint8_t i=0; i<moduleList.size(); i++) {
    (moduleList.get(i))->loop();
  }
}

void ModuleHandler::list() {
  ModulesPrint();
}

bool ModuleHandler::loaded(char *name) {
  for (uint8_t i=0; i<moduleList.size(); i++) {
    if(strcmp(moduleList.get(i)->name(),name) == 0) return true;
  }
  
  return false;
}

PinoccioModule *ModuleHandler::load(char *name) {
  PinoccioModule* module;
  for (uint8_t i=0; i<moduleList.size(); i++) {
    module = moduleList.get(i);
    if(strcmp(module->name(),name) == 0) return module;
  }

  module = ModulesNamed(name);
  if(module) {
    moduleList.add(module);
    module->setup();
  }
  return module;
}

void ModuleHandler::loaded() {
  for (uint8_t i=0; i<moduleList.size(); i++) {
    speol((moduleList.get(i))->name());
  }
}
