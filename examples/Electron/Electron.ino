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
#include <electron.h>

#include "version.h"

void *e_debug(const char *file, int line, const char *function, const char * format, ...)
{
  Serial.print(file);
  Serial.print(" ");
  Serial.println(line);
}

void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  // Add custom setup code here
  e_t e = e_val(NULL,"foo bar, boo baz, \"biz\":424242",0);
  Serial.println(e_count(e));
}

void loop() {
  Scout.loop();
  // Add custom loop code here
}
