/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nwkPrivate.h"

/*****************************************************************************
*****************************************************************************/
enum
{
  NWK_FRAME_STATE_FREE = 0x00,
};

/*****************************************************************************
*****************************************************************************/
static NwkFrame_t nwkFrameFrames[NWK_BUFFERS_AMOUNT];

/*****************************************************************************
*****************************************************************************/
void nwkFrameInit(void)
{
  int i = 0;
  for (i; i < NWK_BUFFERS_AMOUNT; i++)
    nwkFrameFrames[i].state = NWK_FRAME_STATE_FREE;
}

/*****************************************************************************
*****************************************************************************/
NwkFrame_t *nwkFrameAlloc(uint8_t size)
{
  int i = 0;
  for (i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    if (NWK_FRAME_STATE_FREE == nwkFrameFrames[i].state)
    {
      nwkFrameFrames[i].size = sizeof(NwkFrameHeader_t) + size;
      return &nwkFrameFrames[i];
    }
  }
  return NULL;
}

/*****************************************************************************
*****************************************************************************/
void nwkFrameFree(NwkFrame_t *frame)
{
  frame->state = NWK_FRAME_STATE_FREE;
}

/*****************************************************************************
*****************************************************************************/
NwkFrame_t *nwkFrameByIndex(uint8_t i)
{
  return &nwkFrameFrames[i];
}

/*****************************************************************************
*****************************************************************************/
void nwkFrameCommandInit(NwkFrame_t *frame)
{
  frame->tx.status = NWK_SUCCESS_STATUS;
  frame->tx.timeout = 0;
  frame->tx.control = 0;
  frame->tx.confirm = NULL;

  frame->data.header.nwkFcf.ackRequest = 0;
  frame->data.header.nwkFcf.securityEnabled = 0;
  frame->data.header.nwkFcf.linkLocal = 0;
  frame->data.header.nwkFcf.reserved = 0;
  frame->data.header.nwkSeq = ++nwkIb.nwkSeqNum;
  frame->data.header.nwkSrcAddr = nwkIb.addr;
  frame->data.header.nwkDstAddr = 0;
  frame->data.header.nwkSrcEndpoint = 0;
  frame->data.header.nwkDstEndpoint = 0;
}