/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_SYS_TIMER_H_
#define _PINOCCIO_SYS_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
typedef enum SYS_TimerMode_t
{
  SYS_TIMER_INTERVAL_MODE,
  SYS_TIMER_PERIODIC_MODE,
} SYS_TimerMode_t;

typedef struct SYS_Timer_t
{
  // Internal data
  struct SYS_Timer_t   *next;
  uint32_t             timeout;

  // Timer parameters
  uint32_t             interval;
  SYS_TimerMode_t      mode;
  void                 (*handler)(struct SYS_Timer_t *timer);
} SYS_Timer_t;

/*****************************************************************************
*****************************************************************************/
void SYS_TimerInit(void);
void SYS_TimerStart(SYS_Timer_t *timer);
void SYS_TimerStop(SYS_Timer_t *timer);
bool SYS_TimerStarted(SYS_Timer_t *timer);
void SYS_TimerTaskHandler(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_TIMER_H_