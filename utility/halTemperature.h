/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_HAL_TEMPERATURE_H_
#define _PINOCCIO_HAL_TEMPERATURE_H_

/*****************************************************************************
*****************************************************************************/
#ifndef HAL_TEMPERATURE_CALIBRATION_OFFSET
#define HAL_TEMPERATURE_CALIBRATION_OFFSET 0.55 // degrees C
#endif

#define HAL_TEMPERATURE_READING_DELAY      500 // µs

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
float HAL_MeasureTemperature(void);
int8_t HAL_MeasureAdcOffset(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_HAL_TEMPERATURE_H_