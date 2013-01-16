/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_HAL_H_
#define _PINOCCIO_HAL_H_

#include "sysTypes.h"

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
void HAL_Init(void);
void HAL_Delay(uint8_t us);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_HAL_H_