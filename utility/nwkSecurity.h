/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_SECURITY_H_
#define _PINOCCIO_NWK_SECURITY_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "sysConfig.h"

#ifdef NWK_ENABLE_SECURITY

/*- Definitions ------------------------------------------------------------*/
#define NWK_SECURITY_MIC_SIZE        4
#define NWK_SECURITY_KEY_SIZE        16
#define NWK_SECURITY_BLOCK_SIZE      16

/*- Prototypes -------------------------------------------------------------*/
void NWK_SetSecurityKey(uint8_t *key);

void nwkSecurityInit(void);
void nwkSecurityProcess(NwkFrame_t *frame, bool encrypt);
void nwkSecurityTaskHandler(void);

#endif // NWK_ENABLE_SECURITY

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_SECURITY_H_
