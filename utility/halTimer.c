/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

/*- Includes ---------------------------------------------------------------*/
#include "hal.h"
#include "halTimer.h"

/*- Definitions ------------------------------------------------------------*/
#define TIMER_PRESCALER     1

/*- Variables --------------------------------------------------------------*/
volatile uint8_t halTimerIrqCount;
static volatile uint8_t halTimerDelayInt;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void HAL_TimerInit(void)
{
  halTimerIrqCount = 0;

  // OCR4A = ((F_CPU / 1000ul) / TIMER_PRESCALER) * HAL_TIMER_INTERVAL;
  // TCCR4B = (1 << WGM42);              // CTC mode
  // TCCR4B |= (1 << CS40);              // Prescaler 
  // TCCR4B |= (1 << CS41);              // Prescaler 
  // TIMSK4 |= (1 << OCIE4A);            // Enable TC4 interrupt
  
  
  //Timer4 Prescaler = 1024; Preload = 124; Actual Interrupt Time = 8 ms
  TCCR4A = 0x80;
  TCCR4B = 0x0D; 
  OCR4AH = 0x00; 
  OCR4AL = 0x7C; 
  TIMSK4 |= (1 << OCIE4A);
  
  //TCCR0A = (1 << WGM00);
  //TCCR0A = TCCR0A & 0b11111000 | 0x01;
}

/*************************************************************************//**
*****************************************************************************/
void HAL_TimerDelay(uint16_t us)
{
  PRAGMA(diag_suppress=Pa082);

  OCR4B = TCNT4 + us;
  if (OCR4B > OCR4A)
    OCR4B -= OCR4A;

  halTimerDelayInt = 0;
  TIMSK4 |= (1 << OCIE4B);
  while (0 == halTimerDelayInt);
  TIMSK4 &= ~(1 << OCIE4B);

  PRAGMA(diag_default=Pa082);
}

/*************************************************************************//**
*****************************************************************************/
ISR(TIMER4_COMPA_vect)
{
  halTimerIrqCount++;
}

/*************************************************************************//**
*****************************************************************************/
ISR(TIMER4_COMPB_vect)
{
  halTimerDelayInt = 1;
}