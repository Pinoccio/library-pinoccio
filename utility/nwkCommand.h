/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_COMMAND_H_
#define _PINOCCIO_NWK_COMMAND_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "sysTypes.h"

/*- Types ------------------------------------------------------------------*/
enum
{
  NWK_COMMAND_ACK                 = 0x00,
  NWK_COMMAND_ROUTE_ERROR         = 0x01,
  NWK_COMMAND_ROUTE_REQUEST       = 0x02,
  NWK_COMMAND_ROUTE_REPLY         = 0x03,
};

typedef struct PACK NwkCommandAck_t
{
  uint8_t    id;
  uint8_t    seq;
  uint8_t    control;
} NwkCommandAck_t;

typedef struct PACK NwkCommandRouteError_t
{
  uint8_t    id;
  uint16_t   srcAddr;
  uint16_t   dstAddr;
  uint8_t    multicast;
} NwkCommandRouteError_t;

typedef struct PACK NwkCommandRouteRequest_t
{
  uint8_t    id;
  uint16_t   srcAddr;
  uint16_t   dstAddr;
  uint8_t    multicast;
  uint8_t    linkQuality;
} NwkCommandRouteRequest_t;

typedef struct PACK NwkCommandRouteReply_t
{
  uint8_t    id;
  uint16_t   srcAddr;
  uint16_t   dstAddr;
  uint8_t    multicast;
  uint8_t    forwardLinkQuality;
  uint8_t    reverseLinkQuality;
} NwkCommandRouteReply_t;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_COMMAND_H_
