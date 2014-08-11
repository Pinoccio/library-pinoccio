/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include "Flash.h"

// FLASH M25PX16 16Mbit flash chip command set
enum {
  FLASH_WREN = 0x06,
  FLASH_WRDI = 0x04,
  FLASH_RDID = 0x9E,
  FLASH_RDSR = 0x05,
  FLASH_WRSR = 0x01,
  FLASH_READ = 0x03,       // read at half frequency
  FLASH_FAST_READ = 0x0B,  // read at full frequency
  FLASH_SSE = 0x20,
  FLASH_SE = 0xD8,
  FLASH_BE = 0xC7,
  FLASH_PP = 0x02,
  FLASH_DP = 0xB9,
  FLASH_RDP = 0xAB,
  FLASH_NOP = 0xFF,

  // status register bits
  FLASH_WIP = 0x01,
  FLASH_WEL = 0x02,
  FLASH_BP0 = 0x04,
  FLASH_BP1 = 0x08,
  FLASH_BP2 = 0x10
};


// currently supported chip
#define FLASH_MFG 0x20
#define FLASH_ID 0x7115

FlashClass::FlashClass(int chipSelectPin, SPIClass &SPIDriver) : SPI(SPIDriver), CS(chipSelectPin) {
  begin(chipSelectPin, SPIDriver);
}

void FlashClass::begin(int chipSelectPin, SPIClass &SPIDriver) {
  digitalWrite(chipSelectPin, HIGH);
  pinMode(chipSelectPin, OUTPUT);

  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);

  this->SPI = SPIDriver;
  this->CS = chipSelectPin;

  this->SPI.begin();
  this->SPI.setBitOrder(MSBFIRST);
  this->SPI.setDataMode(SPI_MODE0);
  this->SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void FlashClass::begin() {
  begin(this->CS, this->SPI);
}

void FlashClass::end() {
  this->SPI.end();
  digitalWrite(this->CS, HIGH);
}

// return true if the chip is supported
bool FlashClass::available(void) {
  digitalWrite(this->CS, HIGH);

  uint8_t manufacturer;
  uint16_t device;
  this->info(&manufacturer, &device);
  return (FLASH_MFG == manufacturer) && (FLASH_ID == device);
}

void FlashClass::info(uint8_t *manufacturer, uint16_t *device) {
  uint32_t start = millis();
  while (this->isBusy()) {
    if (millis() - start > 1000) {
      return;
    }
  }

  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_RDID);
  *manufacturer = this->SPI.transfer(FLASH_NOP);
  uint8_t id_high = this->SPI.transfer(FLASH_NOP);
  uint8_t id_low = this->SPI.transfer(FLASH_NOP);
  *device = (id_high << 8) | id_low;
  digitalWrite(this->CS, HIGH);
}

bool FlashClass::isBusy(void) {
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_RDSR);
  bool busy = 0 != ((FLASH_WIP | FLASH_WEL) & this->SPI.transfer(FLASH_NOP));
  digitalWrite(this->CS, HIGH);
  delay(10);
  return busy;
}

void FlashClass::read(uint32_t address, void *buffer, uint32_t length) {
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_FAST_READ);
  this->SPI.transfer(address >> 16);
  this->SPI.transfer(address >> 8);
  this->SPI.transfer(address);
  this->SPI.transfer(FLASH_NOP); // read dummy byte

  for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
    *p++ = this->SPI.transfer(FLASH_NOP);
  }

  digitalWrite(this->CS, HIGH);
}

void FlashClass::writeEnable(void) {
  while (this->isBusy()) { }
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_WREN);
  digitalWrite(this->CS, HIGH);
  delay(10);
}

void FlashClass::writeDisable(void) {
  while (this->isBusy()) { }
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_WRDI);
  digitalWrite(this->CS, HIGH);
  delay(10);
}

void FlashClass::write(uint32_t address, void *buffer, uint8_t length) {
  while (this->isBusy()) { }

  writeEnable();
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_PP);
  this->SPI.transfer(address >> 16);
  this->SPI.transfer(address >> 8);
  this->SPI.transfer(address);

  for (uint8_t *p = (uint8_t *)buffer; length != 0; --length) {
    this->SPI.transfer(*p++);
  }

  digitalWrite(this->CS, HIGH);
  writeDisable();
}

void FlashClass::subSectorErase(uint32_t address) {
  while (this->isBusy()) { }

  writeEnable();
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_SSE);
  this->SPI.transfer(address >> 16);
  this->SPI.transfer(address >> 8);
  this->SPI.transfer(address);
  digitalWrite(this->CS, HIGH);
  writeDisable();
}

void FlashClass::sectorErase(uint32_t address) {
  while (this->isBusy()) { }

  writeEnable();
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_SE);
  this->SPI.transfer(address >> 16);
  this->SPI.transfer(address >> 8);
  this->SPI.transfer(address);
  digitalWrite(this->CS, HIGH);
  writeDisable();
}

void FlashClass::bulkErase(void) {
  while (this->isBusy()) { }

  writeEnable();
  digitalWrite(this->CS, LOW);
  this->SPI.transfer(FLASH_BE);
  digitalWrite(this->CS, HIGH);
  writeDisable();
}
