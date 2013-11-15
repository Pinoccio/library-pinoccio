/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_SYS_ENCRYPT_H_
#define _PINOCCIO_SYS_ENCRYPT_H_

#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*- Prototypes -------------------------------------------------------------*/
void SYS_EncryptReq(uint8_t *text, uint8_t *key);
void SYS_EncryptConf(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_ENCRYPT_H_
