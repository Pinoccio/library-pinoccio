/**
 * \file time.c
 *
 * \brief Timing routines implementation
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
 * $Id: time.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include "sysTypes.h"
#include "time.h"

/*- Definitions ------------------------------------------------------------*/
#define TIMER_PRESCALER     8
#define APP_TIMER_INTERVAL  1ul // ms

/*- Variables --------------------------------------------------------------*/
static volatile uint32_t appTime;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void appTimeInit(void)
{
  appTime = 0;

  OCR1A = ((F_CPU / 1000ul) / TIMER_PRESCALER) * APP_TIMER_INTERVAL;
  TCCR1B = (1 << WGM12);              // CTC mode
  TCCR1B |= (1 << CS11);              // Prescaler 8
  TIMSK1 |= (1 << OCIE1A);            // Enable TC1 interrupt
}

/*************************************************************************//**
*****************************************************************************/
uint32_t appTimeGet(void)
{
  uint32_t r;

  ATOMIC_SECTION_ENTER
    r = appTime;
  ATOMIC_SECTION_LEAVE

  return r;
}

/*************************************************************************//**
*****************************************************************************/
ISR(TIMER1_COMPA_vect)
{
  appTime++;
}
