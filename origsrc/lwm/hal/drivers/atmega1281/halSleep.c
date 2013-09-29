/**
 * \file halSleep.c
 *
 * \brief ATmega1281 sleep implementation
 *
 * Copyright (C) 2012-2013, Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 * $Id: halSleep.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <stdbool.h>
#include "hal.h"
#include "halSleep.h"

/*- Definitions ------------------------------------------------------------*/
#define SLEEP_TIMER_CLOCK        32768ul
#define SLEEP_TIMER_PRESCALER    1024ul
#define PRESCALED_CLOCK          (SLEEP_TIMER_CLOCK / SLEEP_TIMER_PRESCALER)

/*- Variables --------------------------------------------------------------*/
static volatile bool halSleepTimerEvent;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static inline void halSleepSynchronize(void)
{
  while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCR2AUB) | (1 << TCR2BUB)));
}

/*************************************************************************//**
*****************************************************************************/
void HAL_Sleep(uint32_t interval)
{
  uint32_t ticks;
  uint16_t integer;
  uint8_t fractional;

  ticks = (interval * PRESCALED_CLOCK) / 1000ul;
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

/*************************************************************************//**
*****************************************************************************/
ISR(TIMER2_COMPA_vect)
{
  halSleepTimerEvent = true;
}
