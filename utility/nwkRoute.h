/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_ROUTE_H_
#define _PINOCCIO_NWK_ROUTE_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "sysTypes.h"
#include "nwkRx.h"
#include "nwkFrame.h"

/*- Definitions ------------------------------------------------------------*/
#define NWK_ROUTE_UNKNOWN            0xffff
#define NWK_ROUTE_NON_ROUTING        0x8000

#ifdef NWK_ENABLE_ROUTING

/*- Types ------------------------------------------------------------------*/
typedef struct NWK_RouteTableEntry_t
{
  uint8_t  fixed     : 1;
  uint8_t  multicast : 1;
  uint8_t  reserved  : 2;
  uint8_t  score     : 4;
  uint16_t dstAddr;
  uint16_t nextHopAddr;
  uint8_t  rank;
  uint8_t  lqi;
} NWK_RouteTableEntry_t;

/*- Prototypes -------------------------------------------------------------*/
NWK_RouteTableEntry_t *NWK_RouteFindEntry(uint16_t dst, uint8_t multicast);
NWK_RouteTableEntry_t *NWK_RouteNewEntry(void);
void NWK_RouteFreeEntry(NWK_RouteTableEntry_t *entry);
uint16_t NWK_RouteNextHop(uint16_t dst, uint8_t multicast);
NWK_RouteTableEntry_t *NWK_RouteTable(void);

void nwkRouteInit(void);
void nwkRouteRemove(uint16_t dst, uint8_t multicast);
void nwkRouteFrameReceived(NwkFrame_t *frame);
void nwkRouteFrameSent(NwkFrame_t *frame);
void nwkRoutePrepareTx(NwkFrame_t *frame);
void nwkRouteFrame(NwkFrame_t *frame);
void nwkRouteErrorReceived(NWK_DataInd_t *ind);
void nwkRouteUpdateEntry(uint16_t dst, uint8_t multicast, uint16_t nextHop, uint8_t lqi);

#endif // NWK_ENABLE_ROUTING

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_ROUTE_H_
