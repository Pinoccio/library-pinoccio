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
#include "modules/SerialFlash/SerialFlash.h"
#include "modules/SerialFlash/Flash.h"
extern "C" {
#include "key/key.h"
}

static SerialFlashModule* flashChipAvailable(int8_t csPin=-1);
static numvar flashInitialize(void);
static numvar flashWrite(void);
static numvar flashRead(void);
static numvar flashEraseSubsector(void);
static numvar flashEraseSector(void);
static numvar flashEraseBulk(void);

SerialFlashModule::SerialFlashModule() {
  flash = new FlashClass(csPin, SPI);
}

SerialFlashModule::~SerialFlashModule() {
  if (flash != NULL) {
    delete flash;
  }
}

void SerialFlashModule::setup() {
  addBitlashFunction("serialflash.initialize", (bitlash_function)flashInitialize);
  addBitlashFunction("serialflash.write", (bitlash_function) flashWrite);
  addBitlashFunction("serialflash.read", (bitlash_function) flashRead);
  addBitlashFunction("serialflash.erase.subsector", (bitlash_function) flashEraseSubsector);
  addBitlashFunction("serialflash.erase.sector", (bitlash_function) flashEraseSector);
  addBitlashFunction("serialflash.erase.bulk", (bitlash_function) flashEraseBulk);
}

void SerialFlashModule::loop() { }

const char *SerialFlashModule::name() {
  return "serialflash";
}

static SerialFlashModule* flashChipAvailable(int8_t csPin) {
  SerialFlashModule* sf = (SerialFlashModule*)ModuleHandler::load("serialflash");
  if (sf == NULL) {
    speol("Module serialflash is not loaded");
    return 0;
  }

  uint32_t start = millis();
  if (csPin < 0) { // start up SPI again, but don't set a new chip select pin=
    sf->flash->begin();
  } else {
    sf->flash->begin(csPin, SPI);
  }
  while (!sf->flash->available()) {
    if (millis() - start > 1000) {
      speol("Serial flash chip not found");
      return 0;
    }
  }
  
  return sf;
}

static numvar flashInitialize(void) {
  if (!checkArgs(1, F("usage: serialflash.initialize(csPin)"))) {
    return 0;
  }
  SerialFlashModule* sf;
  if (!(sf = flashChipAvailable(getarg(1)))) {
    return 0;
  }
  sf->flash->end();
  return 1;
}
static numvar flashWrite(void) {
  if (!checkArgs(2, F("usage: serialflash.write(address, \"string\"|integer)"))) {
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

  SerialFlashModule* sf;
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
  if (!checkArgs(1, F("usage: serialflash.read(address)"))) {
    return 0;
  }

  int32_t addr = getarg(1);
  byte dataRead[65];
  int len;

  SerialFlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->read(addr, &dataRead, 65);
  sf->flash->end();

  // if we read an empty section, set it to 0 so it won't print garbage with key.print()
  if (dataRead[0] == 0xFF) {
    dataRead[0] = 0;
  }
  return keyMap((char*)dataRead, 0);
}

static numvar flashEraseSubsector(void) {
  if (!checkArgs(1, F("usage: serialflash.erase.subsector(address)"))) {
    return 0;
  }

  SerialFlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->subSectorErase(getarg(1));
  sf->flash->end();
  return 1;
}

static numvar flashEraseSector(void) {
  if (!checkArgs(1, F("usage: serialflash.erase.sector(address)"))) {
    return 0;
  }

  SerialFlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->sectorErase(getarg(1));
  sf->flash->end();
  return 1;
}

static numvar flashEraseBulk(void) {
  if (!checkArgs(0, F("usage: serialflash.erase.bulk()"))) {
    return 0;
  }

  SerialFlashModule* sf;
  if (!(sf = flashChipAvailable())) {
    return 0;
  }
  sf->flash->bulkErase();
  sf->flash->end();
  return 1;
}