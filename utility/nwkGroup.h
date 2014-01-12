/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_GROUP_H_
#define _PINOCCIO_NWK_GROUP_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "sysTypes.h"

#ifdef NWK_ENABLE_MULTICAST

/*- Definitions ------------------------------------------------------------*/
#define NWK_MULTICAST_HEADER_SIZE    2

/*- Prototypes -------------------------------------------------------------*/
bool NWK_GroupIsMember(uint16_t group);
bool NWK_GroupAdd(uint16_t group);
bool NWK_GroupRemove(uint16_t group);
uint16_t* NWK_GetGroups(void);

void nwkGroupInit(void);

#endif // NWK_ENABLE_MULTICAST

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_GROUP_H_
