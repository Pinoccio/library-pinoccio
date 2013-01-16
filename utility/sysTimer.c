/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#include <stdlib.h>
#include "hal.h"
#include "halTimer.h"
#include "sysTimer.h"

/*****************************************************************************
*****************************************************************************/
static void placeTimer(SYS_Timer_t *timer);

/*****************************************************************************
*****************************************************************************/
static SYS_Timer_t *timers;

/*****************************************************************************
*****************************************************************************/
void SYS_TimerInit(void)
{
  timers = NULL;
}

/*****************************************************************************
*****************************************************************************/
void SYS_TimerStart(SYS_Timer_t *timer)
{
  if (!SYS_TimerStarted(timer))
    placeTimer(timer);
}

/*****************************************************************************
*****************************************************************************/
void SYS_TimerStop(SYS_Timer_t *timer)
{
  SYS_Timer_t *prev = NULL;
  SYS_Timer_t *t = timers;

  for (t = timers; t; t = t->next)
  {
    if (t == timer)
    {
      if (prev)
        prev->next = t->next;
      else
        timers = t->next;

      if (t->next)
        t->next->timeout += timer->timeout;

      break;
    }
    prev = t;
  }
}

/*****************************************************************************
*****************************************************************************/
bool SYS_TimerStarted(SYS_Timer_t *timer)
{
  SYS_Timer_t *t = timers;

  for (t = timers; t; t = t->next)
    if (t == timer)
      return true;
  return false;
}

/*****************************************************************************
*****************************************************************************/
void SYS_TimerTaskHandler(void)
{
  uint32_t elapsed;
  uint8_t cnt;

  if (0 == halTimerIrqCount)
    return;

  ATOMIC_SECTION_ENTER
    cnt = halTimerIrqCount;
    halTimerIrqCount = 0;
  ATOMIC_SECTION_LEAVE

  elapsed = cnt * HAL_TIMER_INTERVAL;

  while (timers && (timers->timeout <= elapsed))
  {
    SYS_Timer_t *timer = timers;

    elapsed -= timers->timeout;
    timers = timers->next;
    if (SYS_TIMER_PERIODIC_MODE == timer->mode)
      placeTimer(timer);
    timer->handler(timer);
  }

  if (timers)
    timers->timeout -= elapsed;
}

/*****************************************************************************
*****************************************************************************/
static void placeTimer(SYS_Timer_t *timer)
{
  if (timers)
  {
    SYS_Timer_t *prev = NULL;
    uint32_t timeout = timer->interval;
    SYS_Timer_t *t = timers;

    for (t = timers; t; t = t->next)
    {
      if (timeout < t->timeout)
      {
         t->timeout -= timeout;
         break;
      }
      else
        timeout -= t->timeout;

      prev = t;
    }

    timer->timeout = timeout;

    if (prev)
    {
      timer->next = prev->next;
      prev->next = timer;
    }
    else
    {
      timer->next = timers;
      timers = timer;
    }
  }
  else
  {
    timer->next = NULL;
    timer->timeout = timer->interval;
    timers = timer;
  }
}