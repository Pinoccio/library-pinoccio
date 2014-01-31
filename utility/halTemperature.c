/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD license as described in license.txt.         *
\**************************************************************************/

#include <stdbool.h>
#include "hal.h"
#include "halTemperature.h"
#include <avr/sleep.h>
#include <util/delay.h>

/*****************************************************************************
*****************************************************************************/
int8_t HAL_MeasureTemperature(void) {
  uint16_t val;

  uint8_t adcsrc = ADCSRC;
  uint8_t adcsrb = ADCSRB;
  uint8_t adcsra = ADCSRA;
  uint8_t admux = ADMUX;

  ADCSRC = 10<<ADSUT0; // set start-up time
  ADCSRB = 1<<MUX5; // set MUX5 first
  ADMUX = (3<<REFS0) + (9<<MUX0); // store new ADMUX, 1.6V AREF // switch ADC on, set prescaler, start conversion
  ADCSRA = (1<<ADEN) + (1<<ADSC) + (4<<ADPS0);
  do
  {} while( (ADCSRA & (1<<ADSC))); // wait for conversion end ADCSRA = 0; // disable the ADC

  val = ADC;

  ADCSRA = adcsra;
  ADCSRB = adcsrb;
  ADCSRC = adcsrc;
  ADMUX = admux;

  return ((int)((1.13 * val - 272.8)));
}
