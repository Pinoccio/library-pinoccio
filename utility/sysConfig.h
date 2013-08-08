/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_SYS_CONFIG_H_
#define _PINOCCIO_SYS_CONFIG_H_

#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
//#include "config.h"

/*- Definitions ------------------------------------------------------------*/
#ifndef NWK_BUFFERS_AMOUNT
#define NWK_BUFFERS_AMOUNT                       5
#endif

#ifndef NWK_DUPLICATE_REJECTION_TABLE_SIZE
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE       10
#endif

#ifndef NWK_DUPLICATE_REJECTION_TTL
#define NWK_DUPLICATE_REJECTION_TTL              1000 // ms
#endif

#ifndef NWK_ROUTE_TABLE_SIZE
#define NWK_ROUTE_TABLE_SIZE                     10
#endif

#ifndef NWK_ROUTE_DEFAULT_SCORE
#define NWK_ROUTE_DEFAULT_SCORE                  3
#endif

#ifndef NWK_ACK_WAIT_TIME
#define NWK_ACK_WAIT_TIME                        1000 // ms
#endif

#ifndef NWK_GROUPS_AMOUNT
#define NWK_GROUPS_AMOUNT                        10
#endif

#ifndef NWK_ROUTE_DISCOVERY_TABLE_SIZE
#define NWK_ROUTE_DISCOVERY_TABLE_SIZE           5
#endif

#ifndef NWK_ROUTE_DISCOVERY_TIMEOUT
#define NWK_ROUTE_DISCOVERY_TIMEOUT              1000 // ms
#endif

#define NWK_ENABLE_ROUTING
#define NWK_ENABLE_SECURITY
#define NWK_ENABLE_MULTICAST
#define NWK_ENABLE_ROUTE_DISCOVERY
//#define NWK_ENABLE_SECURE_COMMANDS

#ifndef SYS_SECURITY_MODE
#define SYS_SECURITY_MODE                        0
#endif

/*- Sanity checks ----------------------------------------------------------*/
#if defined(NWK_ENABLE_SECURITY) && (SYS_SECURITY_MODE == 0)
  #define PHY_ENABLE_AES_MODULE
#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_CONFIG_H_
