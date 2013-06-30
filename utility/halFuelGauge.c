/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD license as described in license.txt.         *
\**************************************************************************/

#include "halFuelGauge.h"

unsigned int HAL_vcellMAX17048G() {
  unsigned int vcell;
  
  vcell = i2cRead16(0x02);
  vcell = vcell >> 4;  // last 4 bits of vcell are nothing
  
  return vcell;
}

float HAL_percentMAX17048G() {
  unsigned int soc;
  float percent;
  
  soc = i2cRead16(0x04);  // Read SOC register of MAX17048G
  percent = (byte) (soc >> 8);  // High byte of SOC is percentage
  percent += ((float)((byte)soc))/256;  // Low byte is 1/256%
  
  return percent;
}

void HAL_configMAX17048G(byte percent) {
  if ((percent >= 32)||(percent == 0))  // Anything 32 or greater will set to 32%
    i2cWrite16(0x9700, 0x0C);
  else
  {
    byte percentBits = 32 - percent;
    i2cWrite16((0x9700 | percentBits), 0x0C);
  }
}

void HAL_qsMAX17048G() {
  i2cWrite16(0x4000, 0x06);  // Write a 0x4000 to the MODE register
}

unsigned int HAL_i2cRead16(unsigned char address) {
  int data = 0;
  
  Wire.beginTransmission(MAX17048G_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(MAX17048G_ADDRESS, 2);
  while (Wire.available() < 2)
    ;
  data = ((int) Wire.read()) << 8;
  data |= Wire.read();
  
  return data;
}

void HAL_i2cWrite16(unsigned int data, unsigned char address) {
  Wire.beginTransmission(MAX17048G_ADDRESS);
  Wire.write(address);
  Wire.write((byte)((data >> 8) & 0x00FF));
  Wire.write((byte)(data & 0x00FF));
  Wire.endTransmission();
}
