/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef _PINOCCIO_HAL_TEMPERATURE_H_
#define _PINOCCIO_HAL_TEMPERATURE_H_

#include <stdint.h>

/*****************************************************************************
*****************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
int8_t HAL_MeasureTemperature(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_HAL_TEMPERATURE_H_
