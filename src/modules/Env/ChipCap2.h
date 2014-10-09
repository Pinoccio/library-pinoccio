/*

chipcap2 - Mosquino/Arduino library for GE's ChipCap2 series temperature/humidity sensor.
(C)2013 Tim Gipson (drmn4ea at google's email service)

NOTE: This library assumes the chip is factory-configured in I2C, Sleep mode (CC2DxxS parts).
Other CC2 variants can be reprogrammed via a special dance within 10ms of startup, but that is 
beyond the scope of this library.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#define CHIPCAP2_DEFAULT_ADDR 0x28

class ChipCap2  {
 public:
  ChipCap2(uint8_t addr = CHIPCAP2_DEFAULT_ADDR);

  boolean present(void);

  void triggerMeasurement(void);

  int16_t readHumidity_int(void);
  int16_t readTemperature_int(void);
  
  float readHumidity(void);
  float readTemperature(void);

 
 private:
  uint8_t _addr;
  uint8_t raw[4];
};
