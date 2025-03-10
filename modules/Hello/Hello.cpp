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

const __FlashStringHelper *HelloModule::name() const {
  return F("hello");
}

// any scoutscript commands
static numvar hello() {
  speol("world");
  return 0;
}

bool HelloModule::enable() {
  Shell.addFunction("hello", hello);
  Serial.println("hello enabled");
}

void HelloModule::loop() { }

