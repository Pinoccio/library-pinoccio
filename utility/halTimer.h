/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_HAL_TIMER_H_
#define _PINOCCIO_HAL_TIMER_H_

/*****************************************************************************
*****************************************************************************/
#define HAL_TIMER_INTERVAL      10ul // ms

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
extern volatile uint8_t halTimerIrqCount;

/*****************************************************************************
*****************************************************************************/
void HAL_TimerInit(void);
void HAL_TimerDelay(uint16_t us);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_HAL_TIMER_H_