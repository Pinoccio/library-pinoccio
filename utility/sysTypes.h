/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_SYS_TYPES_H_
#define _PINOCCIO_SYS_TYPES_H_
#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define PRAGMA(x)

#define PACK __attribute__ ((packed))

#define INLINE static inline __attribute__ ((always_inline))

#define SYS_EnableInterrupts() sei()

#define ATOMIC_SECTION_ENTER   { uint8_t __atomic = SREG; cli();
#define ATOMIC_SECTION_LEAVE   SREG = __atomic; }

#define SYS_DEVICE_SIZE        131072
#define SYS_PAGE_SIZE          256
#define SYS_BOOTLOADER_SIZE    2048

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_TYPES_H_