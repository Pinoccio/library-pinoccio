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
#include "Example.h"

static numvar example_function() {
  speol("42");
  return 0;
}

void ExampleModule::setup() {
  addBitlashFunction("example", (bitlash_function)example_function);
  Serial.println("example module setup");
}

const char *ExampleModule::name() {
  return "example";
}

uint8_t ExampleModule::foo() {
  return 40+2;
}

