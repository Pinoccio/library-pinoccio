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
#include "hello.h"

static numvar hello();

void HelloModule::setup() {
  addBitlashFunction("hello", (bitlash_function)hello);
  Serial.println("hello setup");
}

void HelloModule::loop() { }

const char *HelloModule::name() {
  return "hello";
}

static numvar hello() {
  speol("world");
  return 0;
}
