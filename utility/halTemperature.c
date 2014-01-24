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
static int8_t halAdcOffset;

static inline void sleep (void)
{
    asm volatile("sleep");
}

/*****************************************************************************
*****************************************************************************/
static inline int16_t HAL_AdcMeasure(void) {
  /* dummy cycle */
  do
  {
      sleep();
  }
  while (ADCSRA & (1 << ADSC));

   /* set by ISR */
  return ADC;
}

/*****************************************************************************
*****************************************************************************/
int8_t HAL_MeasureTemperature(void) {
  int32_t val;

  uint8_t adcsrc = ADCSRC;
  uint8_t adcsrb = ADCSRB;
  uint8_t adcsra = ADCSRA;
  uint8_t admux = ADMUX;
  
  ADCSRC = 10 << ADSUT0;
  ADCSRB = (1 << MUX5);
  ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX0); /* reference: 1.6V, input Temp Sensor */
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */

  _delay_us(HAL_TEMPERATURE_READING_DELAY); /* some time to settle */

  ADCSRA |= (1 << ADIF); /* clear flag */
  ADCSRA |= (1 << ADIE);

  // dummy cycle after REF change (suggested by datasheet)
  HAL_AdcMeasure();

  val = HAL_AdcMeasure();

  ADCSRA = adcsra;
  ADCSRB = adcsrb;
  ADCSRC = adcsrc;
  ADMUX = admux;
  return (int)((1.13 * val - 272.8)) + HAL_TEMPERATURE_CALIBRATION_OFFSET - 3;
}
