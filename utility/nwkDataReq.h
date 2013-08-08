/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_DATA_REQ_H_
#define _PINOCCIO_NWK_DATA_REQ_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "sysConfig.h"
#include "sysTypes.h"

/*- Types ------------------------------------------------------------------*/
enum
{
  NWK_OPT_ACK_REQUEST          = 1 << 0,
  NWK_OPT_ENABLE_SECURITY      = 1 << 1,
  NWK_OPT_BROADCAST_PAN_ID     = 1 << 2,
  NWK_OPT_LINK_LOCAL           = 1 << 3,
  NWK_OPT_MULTICAST            = 1 << 4,
};

typedef struct NWK_DataReq_t
{
  // service fields
  void         *next;
  void         *frame;
  uint8_t      state;

  // request parameters
  uint16_t     dstAddr;
  uint8_t      dstEndpoint;
  uint8_t      srcEndpoint;
  uint8_t      options;
#ifdef NWK_ENABLE_MULTICAST
  uint8_t      memberRadius;
  uint8_t      nonMemberRadius;
#endif
  uint8_t      *data;
  uint8_t      size;
  void         (*confirm)(struct NWK_DataReq_t *req);

  // confirmation parameters
  uint8_t      status;
  uint8_t      control;
} NWK_DataReq_t;

/*- Prototypes -------------------------------------------------------------*/
void NWK_DataReq(NWK_DataReq_t *req);

void nwkDataReqInit(void);
void nwkDataReqTaskHandler(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_DATA_REQ_H_
