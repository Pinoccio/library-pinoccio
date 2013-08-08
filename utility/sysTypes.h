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

#define SYS_LW_MESH_ENV

#if defined(__ICCAVR__)
  #include <inavr.h>
  #include <ioavr.h>
  #include <intrinsics.h>
  #include <pgmspace.h>

  #define PACK

  #define PRAGMA(x) _Pragma(#x)

  #define INLINE PRAGMA(inline=forced) static

  #define SYS_EnableInterrupts() __enable_interrupt()

  #define wdt_reset() (__watchdog_reset())

  #define wdt_enable(timeout) do { \
     uint8_t __atomic = SREG; __disable_interrupt(); \
    __watchdog_reset(); \
    WDTCSR |= (1 << WDCE) | (1 << WDE); \
    WDTCSR = (1 << WDE) | timeout; \
    SREG = __atomic; \
  } while (0)

  #define wdt_disable() do { \
    MCUSR = 0; \
    WDTCSR |= (1 << WDCE) | (1 << WDE); \
    WDTCSR = 0x00; \
  } while (0)

  #define ISR(vec) PRAGMA(vector=vec) __interrupt void handler_##vec(void)

  #define ATOMIC_SECTION_ENTER   { uint8_t __atomic = SREG; __disable_interrupt();
  #define ATOMIC_SECTION_LEAVE   SREG = __atomic; }
/*
#elif defined(__ICCARM__)
  #error Unsupported compiler

#elif defined(__ICCAVR32__)
  #error Unsupported compiler
*/
#else
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

#endif

#if defined(HAL_ATMEGA1281)
  #define SYS_DEVICE_SIZE        131072
  #define SYS_PAGE_SIZE          256
  #define SYS_BOOTLOADER_SIZE    2048

#elif defined(__AVR_ATmega128RFA1__)
  #define SYS_DEVICE_SIZE        131072
  #define SYS_PAGE_SIZE          256
  #define SYS_BOOTLOADER_SIZE    2048

#elif defined(__AVR_ATmega256RFR2__)

#elif defined(HAL_ATXMEGA128B1)

#else
  //#error Unknown HAL
#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_TYPES_H_
