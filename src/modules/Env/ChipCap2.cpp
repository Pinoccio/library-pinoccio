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
	
#include "chipcap2.h"
#include <Wire.h>

ChipCap2::ChipCap2(uint8_t addr)
{
	_addr = addr;
}

boolean ChipCap2::present(void)
{
  if (Wire.requestFrom(_addr, (uint8_t)1, (uint8_t)true))
  {
    return true;
  }
  return false;
}

void ChipCap2::triggerMeasurement(void)
{
  // Measurement is requested by sending a Write to the chip's address.
  // Any data payload is optional.
  Wire.beginTransmission(_addr);
  Wire.endTransmission();
  // After triggering measurement, up to 48msec are needed before output data is valid.
  delay(50);
  if(Wire.requestFrom(_addr, (uint8_t)4, (uint8_t)true)==4)
  {
     // The only read supported by the device is a read request to the chip's address itself (there are no separate registers for T/H). So reading into a scratch buffer here rather than read everything twice when both humidity and temperature are desired.
     raw[0] = Wire.read();
     raw[1] = Wire.read();
     raw[2] = Wire.read();
     raw[3] = Wire.read();	
  }
}

int16_t ChipCap2::readHumidity_int(void)
{
  uint16_t value_temp;

  value_temp = raw[0];
  value_temp = (value_temp & 0x3F) << 8;
  value_temp = value_temp | raw[1];
  value_temp = value_temp / 164;
  return value_temp;
}

int16_t ChipCap2::readTemperature_int(void)
{
  int32_t value_temp;
  int16_t ret;
  value_temp = raw[2];
  value_temp = value_temp << 8;
  value_temp = value_temp | raw[3];
  value_temp = value_temp & 0xFFFC;
  value_temp = ((value_temp * 165) >> 16) - 40;
  ret = value_temp;
  return ret;
}


float ChipCap2::readHumidity(void)
{
  uint16_t value_temp;
  float ret;
  value_temp = raw[0];
  value_temp = (value_temp & 0x3F) << 8;
  value_temp = value_temp | raw[1];
  ret = value_temp;
  ret = ret / 163.84;
  return ret;
}

float ChipCap2::readTemperature(void)
{

  uint16_t value_temp;
  float ret;

  value_temp = raw[2];
  value_temp = value_temp << 8;
  value_temp = value_temp | raw[3];
  value_temp = value_temp & 0xFFFC;
  ret = value_temp;
  ret = ((ret / 65536) * 165) - 40;
  return ret;
}

 


