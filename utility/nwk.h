/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_H_
#define _PINOCCIO_NWK_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "sysConfig.h"
#include "nwkRoute.h"
#include "nwkGroup.h"
#include "nwkSecurity.h"
#include "nwkDataReq.h"

/*- Definitions ------------------------------------------------------------*/
#define NWK_MAX_PAYLOAD_SIZE            (127 - 16/*NwkFrameHeader_t*/ - 2/*crc*/)

#define NWK_BROADCAST_PANID             0xffff
#define NWK_BROADCAST_ADDR              0xffff

#define NWK_ENDPOINTS_AMOUNT            16

/*- Types ------------------------------------------------------------------*/
typedef enum
{
  NWK_SUCCESS_STATUS                      = 0x00,
  NWK_ERROR_STATUS                        = 0x01,
  NWK_OUT_OF_MEMORY_STATUS                = 0x02,

  NWK_NO_ACK_STATUS                       = 0x10,
  NWK_NO_ROUTE_STATUS                     = 0x11,

  NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS   = 0x20,
  NWK_PHY_NO_ACK_STATUS                   = 0x21,
} NWK_Status_t;

typedef struct NwkIb_t
{
  uint16_t     addr;
  uint16_t     panId;
  uint8_t      nwkSeqNum;
  uint8_t      macSeqNum;
  bool         (*endpoint[NWK_ENDPOINTS_AMOUNT])(NWK_DataInd_t *ind);
#ifdef NWK_ENABLE_SECURITY
  uint32_t     key[4];
#endif
} NwkIb_t;

/*- Variables --------------------------------------------------------------*/
extern NwkIb_t nwkIb;

/*- Prototypes -------------------------------------------------------------*/
void NWK_Init(void);
void NWK_SetAddr(uint16_t addr);
void NWK_SetPanId(uint16_t panId);
void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind));
bool NWK_Busy(void);
void NWK_SleepReq(void);
void NWK_WakeupReq(void);
void NWK_TaskHandler(void);

uint8_t NWK_LinearizeLqi(uint8_t lqi);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_H_

