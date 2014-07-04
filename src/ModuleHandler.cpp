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

void PinoccioModuleHandler::add(PinoccioModule* module) {
  moduleList.add(module);
}

void PinoccioModuleHandler::setup() {
  for (uint8_t i=0; i<moduleList.size(); i++) {
    (moduleList.get(i))->setup();
  }
}

void PinoccioModuleHandler::loop() {
  for (uint8_t i=0; i<moduleList.size(); i++) {
    (moduleList.get(i))->loop();
  }
}