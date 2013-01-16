/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_SYS_H_
#define _PINOCCIO_SYS_H_

#include "sysConfig.h"
#include "phy.h"
#include "nwk.h"
#include "hal.h"

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
void SYS_Init(void);
void SYS_TaskHandler(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_H_