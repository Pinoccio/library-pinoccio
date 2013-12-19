/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* Derived from Jim Lindblom @ SparkFun, licensed (CC BY-SA 3.0)            *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_HAL_FUEL_GAUGE_H_
#define _PINOCCIO_HAL_FUEL_GAUGE_H_

#include "Arduino.h"

/*****************************************************************************
*****************************************************************************/
#define MAX17048G_ADDRESS 0x36  // R/W =~ 0x6D/0x6C

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
/*
vcellMAX17048G() returns a 12-bit ADC reading of the battery voltage,
as reported by the MAX17048G's VCELL register.
This does not return a voltage value. To convert this to a voltage,
multiply by 5 and divide by 4096.
*/
int HAL_FuelGaugeVoltage();

/*
percentMAX17048G() returns an int value of the battery percentage
reported from the SOC register of the MAX17048G.
*/
int HAL_FuelGaugePercent();

/*
configMAX17048G(byte percent) configures the config register of
the MAX170143, specifically the alert threshold therein. Pass a
value between 1 and 32 to set the alert threshold to a value between
1 and 32%. Any other values will set the threshold to 32%.
*/
void HAL_FuelGaugeConfig(byte percent);

/*
qsMAX17048G() issues a quick-start command to the MAX17048G.
A quick start allows the MAX17048G to restart fuel-gauge calculations
in the same manner as initial power-up of the IC. If an application's
power-up sequence is very noisy, such that excess error is introduced
into the IC's first guess of SOC, you can issue a quick-start
to reduce the error.
*/
void HAL_FuelGaugeQuickStart();

/*
i2cRead16(unsigned char address) reads a 16-bit value beginning
at the 8-bit address, and continuing to the next address. A 16-bit
value is returned.
*/
unsigned int HAL_FuelGaugei2cRead16(unsigned char address);

/*
i2cWrite16(unsigned int data, unsigned char address) writes 16 bits
of data beginning at an 8-bit address, and continuing to the next.
*/
void HAL_FuelGaugei2cWrite16(unsigned int data, unsigned char address);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_HAL_FUEL_GAUGE_H_