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
#include <peripherals/Flash.h>

#include "version.h"

FlashClass Flash(SS, SPI);

#define BUFSIZE 32
int ctr = 0;

byte dataToWrite[BUFSIZE];
byte dataWritten[BUFSIZE];

void setup() {
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  // Add custom setup code here
  
  if (!Scout.isLeadScout()) {
    Serial.print("This sketch can only be run on a lead scout");
    return;
  }
  
  // initialize the flash chip and ensure it can be found on the SPI bus
  uint32_t start = millis(); 
  Flash.begin(SS, SPI);
  while (!Flash.available()) {
    if (millis() - start > 1000) {
      Serial.println("FAIL: Serial flash chip not found");
      return;
    }
  }
  
  // set some defaults for writing
  uint32_t addr = 0x00000;
  const char* sample = "test string";
  
  // copy the sample string into our BUFSIZE-sized byte array
  strncpy((char*)dataToWrite, sample, strlen(sample));
  
  // write the byte array to flash as a batched-write size of BUFSIZE
  Serial.println("- Writing test string to flash");
  Flash.write(addr, &dataToWrite, BUFSIZE);

  // read back what was written to flash, into a second byte array to confirm write was successful
  Serial.println("- Reading back memory from flash");
  Flash.read(addr, &dataWritten, BUFSIZE);
 
  // they should be equal arrays
  Serial.print("- Confirming test string was written successfully: ");
  if (strncmp((const char*)dataToWrite, (const char*)dataWritten, BUFSIZE) != 0) {
    
    // if not, let's spill what we wrote vs what we read back to see what happened
    Serial.println("No");
    Serial.print("FAIL: Data failed to write to and read from flash at address: ");
    Serial.println(addr, HEX);
    
    dataToWrite[ctr] = 0;
    dataWritten[ctr] = 0;
  
    Serial.print("dataToWrite: ");
    for (int i=0; i<BUFSIZE; i++) {
      Serial.print(dataToWrite[i], HEX);
    }
    Serial.println();
    Serial.print("dataWritten: ");
    for (int i=0; i<BUFSIZE; i++) {
      Serial.print(dataWritten[i], HEX);
    }
    Serial.println();
  } else {
    // write and read back was successful!
    Serial.println("Yes!");
  }     
  Serial.println("Done");
}

void loop() {
  Scout.loop();
  // Add custom loop code here
}
