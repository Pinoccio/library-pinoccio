/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef _PINOCCIO_FLASH_H
#define _PINOCCIO_FLASH_H

#include <SPI.h>

class FlashClass {
private:
  SPIClass &SPI;
  int CS;
public:
  bool available(void);
  void info(uint8_t *maufacturer, uint16_t *device);
  void read(uint32_t address, void *buffer, uint32_t length);
  bool isBusy(void);
  void writeEnable(void);
  void writeDisable(void);
  void write(uint32_t address, void *buffer, uint8_t length);
  void subSectorErase(uint32_t address);
  void sectorErase(uint32_t address);
  void bulkErase(void);

  void begin(int chipSelectPin, SPIClass &SPIDriver);
  void begin();
  void end();

  FlashClass(int chipSelectPin, SPIClass &SPIDriver);
};

#endif