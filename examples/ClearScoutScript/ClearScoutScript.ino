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
#include <EEPROM.h>

#include "version.h"

void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  // Add custom setup code here
  
  Led.cyan();
  
  // write 0xFF to all ScoutScript bytes of EEPROM
  for (int i = 0; i < ENDDB; i++) {
    EEPROM.write(i, 0xFF);
  }
    
  // turn the LED on when we're done
  Led.green();
}

void loop() {
  Scout.loop();
  // Add custom loop code here
}
