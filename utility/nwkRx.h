/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_RX_H_
#define _PINOCCIO_NWK_RX_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "sysTypes.h"
#include "nwkFrame.h"

/*- Types ------------------------------------------------------------------*/
enum
{
  NWK_IND_OPT_ACK_REQUESTED     = 1 << 0,
  NWK_IND_OPT_SECURED           = 1 << 1,
  NWK_IND_OPT_BROADCAST         = 1 << 2,
  NWK_IND_OPT_LOCAL             = 1 << 3,
  NWK_IND_OPT_BROADCAST_PAN_ID  = 1 << 4,
  NWK_IND_OPT_LINK_LOCAL        = 1 << 5,
  NWK_IND_OPT_MULTICAST         = 1 << 6,
};

typedef struct NWK_DataInd_t
{
  uint16_t     srcAddr;
  uint16_t     dstAddr;
  uint8_t      srcEndpoint;
  uint8_t      dstEndpoint;
  uint8_t      options;
  uint8_t      *data;
  uint8_t      size;
  uint8_t      lqi;
  int8_t       rssi;
} NWK_DataInd_t;

/*- Prototypes -------------------------------------------------------------*/
void NWK_SetAckControl(uint8_t control);

#ifdef NWK_ENABLE_ADDRESS_FILTER
bool NWK_FilterAddress(uint16_t addr, uint8_t *lqi);
#endif

void nwkRxInit(void);
void nwkRxDecryptConf(NwkFrame_t *frame, bool status);
void nwkRxTaskHandler(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _NWK_RX_H_
