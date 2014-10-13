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
#include "Flash.h"

using namespace pinoccio;

FlashModule FlashModule::instance;

const __FlashStringHelper *FlashModule::name() const {
  return F("flash");
}


FlashModule* flashChipAvailable() {
  FlashModule* sf = &(FlashModule::instance);
  if(!sf->flash)
  {
    speol(F("not initialized"));
    return 0;
  }

  uint32_t start = millis();
  sf->flash->begin();
  while (!sf->flash->available()) {
    if (millis() - start > 1000) {
      speol(F("flash chip not found"));
      return 0;
    }
  }
  
  return sf;
}

static numvar flashInitialize(void) {
  if (!checkArgs(1, F("usage: flash.initialize(csPin)"))) {
    return 0;
  }
  FlashModule* sf = &(FlashModule::instance);
  if(sf->flash) delete sf->flash;
  sf->flash = new FlashClass(getarg(1), SPI);

  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->end();
  return 1;
}

static numvar flashWrite(void) {
  if (!checkArgs(2, F("usage: flash.write(address, \"string\"|integer)"))) {
    return 0;
  }

  int32_t addr = getarg(1);
  byte dataToWrite[SERIAL_FLASH_BUFSIZE];
  byte dataWritten[SERIAL_FLASH_BUFSIZE];
  int len;

  if (isstringarg(2)) {
    // copy the sample string into our SERIAL_FLASH_BUFSIZE-sized byte array
    len = strlen((char*)getstringarg(2));
    if (len > SERIAL_FLASH_BUFSIZE-1) {
      speol("Can write maximum string length of 64 bytes");
      return 0;
    }
    strncpy((char*)dataToWrite, (char*)getstringarg(2), len);
  } else {
    // convert the the int into our SERIAL_FLASH_BUFSIZE-sized byte array via itoa
    ltoa(getarg(2), (char*)dataToWrite, 10);
    len = strlen((char*)dataToWrite);
  }
  dataToWrite[len+1] = 0;

  FlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  // write the byte array to flash as a batched-write size of SERIAL_FLASH_BUFSIZE
  //Serial.println("- Writing test string to flash");
  sf->flash->write(addr, &dataToWrite, len+1);

  // read back what was written to flash, into a second byte array to confirm write was successful
  //Serial.println("- Reading back memory from flash");
  sf->flash->read(addr, &dataWritten, len+1);
  sf->flash->end();

  // they should be equal arrays
  //Serial.print("- Confirming test string was written successfully: ");
  if (strncmp((const char*)dataToWrite, (const char*)dataWritten, len) != 0) {
    return 0;
    // if not, let's spill what we wrote vs what we read back to see what happened
    /* DEBUG OUTPUT
    Serial.println("No");
    Serial.print("FAIL: Data failed to write to and read from flash at address: ");
    Serial.println(addr, HEX);

    dataToWrite[ctr] = 0;
    dataWritten[ctr] = 0;

    Serial.print("dataToWrite: ");
    for (int i=0; i<SERIAL_FLASH_BUFSIZE; i++) {
      Serial.print(dataToWrite[i], HEX);
    }
    Serial.println();
    Serial.print("dataWritten: ");
    for (int i=0; i<SERIAL_FLASH_BUFSIZE; i++) {
      Serial.print(dataWritten[i], HEX);
    }
    Serial.println();
    */
  } else {
    // write and read back was successful!
    //Serial.println("Yes!");
    return 1;
  }
}

static numvar flashRead(void) {
  if (!checkArgs(1, F("usage: flash.read(address)"))) {
    return 0;
  }

  int32_t addr = getarg(1);
  byte dataRead[65];
  int len;

  FlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->read(addr, &dataRead, 65);
  sf->flash->end();

  // if we read an empty section, set it to 0 so it won't print garbage
  if (dataRead[0] == 0xFF) {
    dataRead[0] = 0;
  }
  sp((char*)dataRead);
  return 1;
}

static numvar flashEraseSubsector(void) {
  if (!checkArgs(1, F("usage: flash.erase.subsector(address)"))) {
    return 0;
  }

  FlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->subSectorErase(getarg(1));
  sf->flash->end();
  return 1;
}

static numvar flashEraseSector(void) {
  if (!checkArgs(1, F("usage: flash.erase.sector(address)"))) {
    return 0;
  }

  FlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->sectorErase(getarg(1));
  sf->flash->end();
  return 1;
}

static numvar flashEraseBulk(void) {
  if (!checkArgs(0, F("usage: flash.erase.bulk()"))) {
    return 0;
  }

  FlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->bulkErase();
  sf->flash->end();
  return 1;
}

bool FlashModule::enable() {
  Shell.addFunction("flash.initialize", flashInitialize);
  Shell.addFunction("flash.write", flashWrite);
  Shell.addFunction("flash.read", flashRead);
  Shell.addFunction("flash.erase.subsector", flashEraseSubsector);
  Shell.addFunction("flash.erase.sector", flashEraseSector);
  Shell.addFunction("flash.erase.bulk", flashEraseBulk);
}

void FlashModule::loop() { }

