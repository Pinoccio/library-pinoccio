// Copyright 2013 Pervasive Displays, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied.  See the License for the specific language
// governing permissions and limitations under the License.

#if !defined(EPD_FLASH_H)
#define EPD_FLASH_H 1

#include <Arduino.h>
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

	// inline static void attachInterrupt();
	// inline static void detachInterrupt();

	void begin(int chipSelectPin, SPIClass &SPIDriver);
	void end();

	FlashClass(int chipSelectPin, SPIClass &SPIDriver);

};

extern FlashClass Flash;

#endif


