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
#include "modules/PinoccioModule.h"

PinoccioModule::PinoccioModule() {
  bridge = false;
  backpack = false;
  ModuleHandler::add(this);
}

PinoccioModule::~PinoccioModule() { }

void PinoccioModule::setBackpack(bool isBackpack) {
  backpack = isBackpack;
}

void PinoccioModule::setBridge(bool isBridge) {
  bridge = isBridge;
}

bool PinoccioModule::isBackpack() {
  return backpack;
}

bool PinoccioModule::isBridge() {
  return bridge;
}

void PinoccioModule::setup() { }

void PinoccioModule::loop() { }