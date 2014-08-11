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
#include "Hello.h"

using namespace pinoccio;

HelloModule HelloModule::instance;

// NOTE: this is an example for adding modules into the main library, see examples/ModuleDemo/ for adding them to any sketch

static numvar hello();

bool HelloModule::load() {
  Shell.addFunction("hello", hello);
  Serial.println("hello loaded");
  return true;
}

void HelloModule::loop() { }

const __FlashStringHelper *HelloModule::name() const {
  return F("hello");
}

static numvar hello() {
  speol("world");
  return 0;
}
