/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#include "sysTypes.h"
#include "hal.h"
#include "halTimer.h"

/*****************************************************************************
*****************************************************************************/
void HAL_Init(void)
{
  MCUSR = 0;
  wdt_disable();
  SYS_EnableInterrupts();

  HAL_TimerInit();
}

/*****************************************************************************
*****************************************************************************/
void HAL_Delay(uint8_t us)
{
  HAL_TimerDelay(us);
}