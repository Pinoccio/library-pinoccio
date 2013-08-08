/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_FRAME_H_
#define _PINOCCIO_NWK_FRAME_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "sysTypes.h"

/*- Definitions ------------------------------------------------------------*/
#define NWK_FRAME_MAX_PAYLOAD_SIZE   127

/*- Types ------------------------------------------------------------------*/
typedef struct PACK NwkFrameHeader_t
{
  uint16_t    macFcf;
  uint8_t     macSeq;
  uint16_t    macDstPanId;
  uint16_t    macDstAddr;
  uint16_t    macSrcAddr;

  struct PACK
  {
    uint8_t   ackRequest : 1;
    uint8_t   security   : 1;
    uint8_t   linkLocal  : 1;
    uint8_t   multicast  : 1;
    uint8_t   reserved   : 4;
  }           nwkFcf;
  uint8_t     nwkSeq;
  uint16_t    nwkSrcAddr;
  uint16_t    nwkDstAddr;
  struct PACK
  {
    uint8_t   nwkSrcEndpoint : 4;
    uint8_t   nwkDstEndpoint : 4;
  };
} NwkFrameHeader_t;

typedef struct PACK NwkFrameMulticastHeader_t
{
  uint16_t    nonMemberRadius    : 4;
  uint16_t    maxNonMemberRadius : 4;
  uint16_t    memberRadius       : 4;
  uint16_t    maxMemberRadius    : 4;
} NwkFrameMulticastHeader_t;

typedef struct NwkFrame_t
{
  uint8_t      state;
  uint8_t      size;

  union
  {
    NwkFrameHeader_t header;
    uint8_t          data[NWK_FRAME_MAX_PAYLOAD_SIZE];
  };

  uint8_t      *payload;

  union
  {
    struct
    {
      uint8_t  lqi;
      int8_t   rssi;
    } rx;

    struct
    {
      uint8_t  status;
      uint16_t timeout;
      uint8_t  control;
      void     (*confirm)(struct NwkFrame_t *frame);
    } tx;
  };
} NwkFrame_t;

/*- Prototypes -------------------------------------------------------------*/
void nwkFrameInit(void);
NwkFrame_t *nwkFrameAlloc(void);
void nwkFrameFree(NwkFrame_t *frame);
NwkFrame_t *nwkFrameNext(NwkFrame_t *frame);
void nwkFrameCommandInit(NwkFrame_t *frame);

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static inline uint8_t nwkFramePayloadSize(NwkFrame_t *frame)
{
  return frame->size - (frame->payload - frame->data);
}

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_FRAME_H_
