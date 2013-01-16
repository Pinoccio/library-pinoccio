/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#include <stdbool.h>
#include "hal.h"
#include "halSleep.h"

/*****************************************************************************
*****************************************************************************/
#define HAL_SLEEP_TIMER_CLOCK        32768ul
#define HAL_SLEEP_TIMER_PRESCALER    1024ul

/*****************************************************************************
*****************************************************************************/
static volatile bool halSleepTimerEvent;

/*****************************************************************************
*****************************************************************************/
inline void halSleepSynchronize(void)
{
  while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCR2AUB) | (1 << TCR2BUB)));
}

/*****************************************************************************
*****************************************************************************/
void HAL_Sleep(uint32_t interval)
{
  uint32_t ticks;
  uint16_t integer;
  uint8_t fractional;

  ticks = (HAL_SLEEP_TIMER_CLOCK * interval) / (HAL_SLEEP_TIMER_PRESCALER * 1000ul);
  if (0 == ticks)
    return;

  integer = ticks >> 8;
  fractional = ticks & 0xff;

  TIMSK2 = 0;
  ASSR |= (1 << AS2);
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  halSleepSynchronize();

  TIFR2 = (1 << OCF2B) | (1 << OCF2A) | (1 << TOV2);
  TCCR2B = ((1 << CS20) | (1 << CS21) | (1 << CS22));   // clk/1024

  while (1)
  {
    halSleepTimerEvent = false;

    if (integer > 0)
      OCR2A = 0xff;
    else if (fractional > 0)
      OCR2A = fractional;
    else
    {
      halSleepSynchronize();
      TIMSK2 = 0;
      TCCR2B = 0;
      GTCCR |= (1 << PSRASY);
      halSleepSynchronize();
      ASSR &= ~(1 << AS2);
      return;
    }

    TIMSK2 |= (1 << OCIE2A);

    halSleepSynchronize();

    SMCR = (1 << SM1) | (1 << SM0) | (1 << SE); // power-save
    asm("sleep");
    SMCR = 0;

    bool event;

    ATOMIC_SECTION_ENTER
      event = halSleepTimerEvent;
    ATOMIC_SECTION_LEAVE

    if (event)
    {
      if (integer > 0)
        integer--;
      else
        fractional = 0;
    }
    else
    {
      // TODO: wakeup from some other source
    }
  }
}

/*****************************************************************************
*****************************************************************************/
ISR(TIMER2_COMPA_vect)
{
  halSleepTimerEvent = true;
}