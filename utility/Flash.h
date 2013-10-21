/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the Apache license as described in license.txt.      *
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
	void read(uint32_t address, void *buffer, uint16_t length);
	bool isBusy(void);
	void writeEnable(void);
	void writeDisable(void);
	void write(uint32_t address, void *buffer, uint16_t length);
	void sectorErase(uint32_t address);
	void bulkErase(void);

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	void begin(int chipSelectPin, SPIClass &SPIDriver);
	void end();

	FlashClass(int chipSelectPin, SPIClass &SPIDriver);

};

extern FlashClass Flash;

#endif


