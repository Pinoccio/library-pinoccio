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
static volatile uint8_t halAdcFinished;
static int8_t halAdcOffset;

/*****************************************************************************
*****************************************************************************/
static inline int16_t HAL_AdcMeasure(void)
{
  set_sleep_mode(SLEEP_MODE_ADC);
  /* dummy cycle */
  halAdcFinished = false;
  do
  {
    sleep_mode();
    /* sleep, wake up by ADC IRQ */
    /* check here for ADC IRQ because sleep mode could wake up from * another source too */
  }
  while (false == halAdcFinished);
  /* set by ISR */
  return ADC;
}

/*****************************************************************************
*****************************************************************************/
float HAL_MeasureTemperature(void)
{
  int32_t val = 0;

  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
  ADCSRB = (1 << MUX5);
  ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3) | (1 << MUX0); /* reference: 1.6V, input Temp Sensor */

  _delay_us(HAL_TEMPERATURE_READING_DELAY); /* some time to settle */

  ADCSRA |= (1 << ADIF); /* clear flag */
  ADCSRA |= (1 << ADIE);

  /* dummy cycle after REF change (suggested by datasheet) */
  HAL_AdcMeasure();

  _delay_us(HAL_TEMPERATURE_READING_DELAY / 2); /* some time to settle */

  val = HAL_AdcMeasure() - halAdcOffset;

  ADCSRA &= ~((1 << ADEN) | (1 << ADIE));

  return ((1.13 * val - 272.8)) + HAL_TEMPERATURE_CALIBRATION_OFFSET - 3;
}

/*****************************************************************************
*****************************************************************************/
int8_t HAL_MeasureAdcOffset(void)
{
  uint16_t val;

  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); /* PS 64 */
  ADCSRB = 0;
  ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << MUX3); /* reference: 1.6V, differential ADC0-ADC0 10x */

  _delay_us(HAL_TEMPERATURE_READING_DELAY); /* some time to settle */

  ADCSRA |= (1 << ADIF); /* clear flag */
  ADCSRA |= (1 << ADIE);

  /* dummy cycle after REF change (suggested by datasheet) */
  HAL_AdcMeasure();

  _delay_us(HAL_TEMPERATURE_READING_DELAY / 2); /* some time to settle */

  val = HAL_AdcMeasure();

  ADCSRA &= ~((1 << ADEN) | (1 << ADIE));

  halAdcOffset = val;
  return (val);
}

/*****************************************************************************
*****************************************************************************/
ISR(ADC_vect, ISR_BLOCK)
{
  halAdcFinished = true;
}