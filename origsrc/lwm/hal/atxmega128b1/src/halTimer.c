/**
 * \file halTimer.c
 *
 * \brief ATxmega128b1 timer implementation
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
 * $Id: halTimer.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "hal.h"
#include "halTimer.h"

/*- Definitions ------------------------------------------------------------*/
#define TIMER_PRESCALER     8

/*- Variables --------------------------------------------------------------*/
volatile uint8_t halTimerIrqCount;
static volatile uint8_t halTimerDelayInt;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void HAL_TimerInit(void)
{
  halTimerIrqCount = 0;

  TCC1.PER = ((F_CPU / 1000ul) / TIMER_PRESCALER) * HAL_TIMER_INTERVAL;
  TCC1.CTRLB = TC_WGMODE_NORMAL_gc;
  TCC1.CTRLA = TC_CLKSEL_DIV8_gc;
  TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
}

/*************************************************************************//**
*****************************************************************************/
void HAL_TimerDelay(uint16_t us)
{
  PRAGMA(diag_suppress=Pa082);

#if F_CPU == 4000000
  us >>= 1;
#elif F_CPU == 8000000
  // Empty
#elif F_CPU == 12000000
  us += (us >> 1);
#elif F_CPU == 16000000
  us <<= 1;
#elif F_CPU == 32000000
  us <<= 2;
#endif

  TCC1.CCB = TCC1.CNT + us;
  if (TCC1.CCB > TCC1.PER)
    TCC1.CCB -= TCC1.PER;

  halTimerDelayInt = 0;
  TCC1.INTCTRLB = TC_CCBINTLVL_LO_gc;
  while (0 == halTimerDelayInt);
  TCC1.INTCTRLB = TC_CCBINTLVL_OFF_gc;

  PRAGMA(diag_default=Pa082);
}

/*************************************************************************//**
*****************************************************************************/
ISR(TCC1_OVF_vect)
{
  halTimerIrqCount++;
}

/*************************************************************************//**
*****************************************************************************/
ISR(TCC1_CCB_vect)
{
  halTimerDelayInt = 1;
}
