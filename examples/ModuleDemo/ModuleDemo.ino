/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2014, Pinoccio Inc. All rights reserved.                   *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the MIT License as described in license.txt.         *
\**************************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>

// include the module header file
#include "./Example.h"

void setup() {
  Scout.setup("Module");

  // get it dynamically, this will always return the same module instance (is a singleton)
  ExampleModule *example = (ExampleModule*)ModuleHandler::load("example");
  Serial.println(example->foo());
}

void loop() {
  Scout.loop();
}
