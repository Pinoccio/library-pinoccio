/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#include "sysConfig.h"
#include "phy.h"
#include "nwk.h"
#include "hal.h"
#include "sysTimer.h"

/*****************************************************************************
*****************************************************************************/
void SYS_Init(void)
{
  HAL_Init();
  SYS_TimerInit();
  PHY_Init();
  NWK_Init();
}

/*****************************************************************************
*****************************************************************************/
void SYS_TaskHandler(void)
{
  PHY_TaskHandler();
  NWK_TaskHandler();
  SYS_TimerTaskHandler();
}