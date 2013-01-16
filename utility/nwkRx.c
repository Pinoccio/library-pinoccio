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
#include "phy.h"
#include "nwk.h"
#include "nwkPrivate.h"
#include "sysTimer.h"

/*****************************************************************************
*****************************************************************************/
#define NWK_RX_DUPLICATE_REJECTION_TIMER_INTERVAL   20 // ms
#define DUPLICATE_REJECTION_TTL \
            ((NWK_DUPLICATE_REJECTION_TTL / NWK_RX_DUPLICATE_REJECTION_TIMER_INTERVAL) + 1)
#define NWK_SERVICE_ENDPOINT_ID    0

/*****************************************************************************
*****************************************************************************/
enum
{
  NWK_RX_STATE_RECEIVED      = 0x20,
  NWK_RX_STATE_DECRYPT       = 0x21,
  NWK_RX_STATE_INDICATE      = 0x22,
  NWK_RX_STATE_ROUTE         = 0x23,
  NWK_RX_STATE_FINISH        = 0x24,
};

typedef struct NwkDuplicateRejectionRecord_t
{
  uint16_t   src;
  uint8_t    seq;
  uint16_t   ttl;
} NwkDuplicateRejectionRecord_t;

/*****************************************************************************
*****************************************************************************/
static void nwkRxSendAckConf(NwkFrame_t *frame);
static void nwkRxDuplicateRejectionTimerHandler(SYS_Timer_t *timer);
static bool nwkRxSeriveDataInd(NWK_DataInd_t *ind);

/*****************************************************************************
*****************************************************************************/
static NwkDuplicateRejectionRecord_t nwkRxDuplicateRejectionTable[NWK_DUPLICATE_REJECTION_TABLE_SIZE];
static uint8_t nwkRxActiveFrames;
static uint8_t nwkRxAckControl;
static SYS_Timer_t nwkRxDuplicateRejectionTimer;

/*****************************************************************************
*****************************************************************************/
void NWK_SetAckControl(uint8_t control)
{
  nwkRxAckControl = control;
}

/*****************************************************************************
*****************************************************************************/
void nwkRxInit(void)
{
  uint8_t i = 0;
  for (i = 0; i < NWK_DUPLICATE_REJECTION_TABLE_SIZE; i++)
    nwkRxDuplicateRejectionTable[i].ttl = 0;

  nwkRxActiveFrames = 0;

  nwkRxDuplicateRejectionTimer.interval = NWK_RX_DUPLICATE_REJECTION_TIMER_INTERVAL;
  nwkRxDuplicateRejectionTimer.mode = SYS_TIMER_INTERVAL_MODE;
  nwkRxDuplicateRejectionTimer.handler = nwkRxDuplicateRejectionTimerHandler;

  NWK_OpenEndpoint(NWK_SERVICE_ENDPOINT_ID, nwkRxSeriveDataInd);
}

/*****************************************************************************
*****************************************************************************/
void PHY_DataInd(PHY_DataInd_t *ind)
{
  NwkFrame_t *frame;

  if (0x88 != ind->data[1] || (0x61 != ind->data[0] && 0x41 != ind->data[0]) ||
      ind->size < sizeof(NwkFrameHeader_t))
    return;

  if (NULL == (frame = nwkFrameAlloc(ind->size - sizeof(NwkFrameHeader_t))))
    return;

  frame->state = NWK_RX_STATE_RECEIVED;
  frame->rx.lqi = ind->lqi;
  frame->rx.rssi = ind->rssi;

  memcpy((uint8_t *)&frame->data, ind->data, ind->size);

  ++nwkRxActiveFrames;
}

/*****************************************************************************
*****************************************************************************/
static void nwkRxSendAck(NwkFrame_t *frame)
{
  NwkFrame_t *ack;
  NwkAckCommand_t *command;

  if (NULL == (ack = nwkFrameAlloc(sizeof(NwkAckCommand_t))))
    return;

  nwkFrameCommandInit(ack);

  ack->tx.confirm = nwkRxSendAckConf;

  ack->data.header.nwkDstAddr = frame->data.header.nwkSrcAddr;

  command = (NwkAckCommand_t *)ack->data.payload;

  command->id = NWK_COMMAND_ACK;
  command->control = nwkRxAckControl;
  command->seq = frame->data.header.nwkSeq;

  nwkTxFrame(ack);
}

/*****************************************************************************
*****************************************************************************/
static void nwkRxSendAckConf(NwkFrame_t *frame)
{
  nwkFrameFree(frame);
}

/*****************************************************************************
*****************************************************************************/
bool nwkRxBusy(void)
{
  return nwkRxActiveFrames > 0;
}

#ifdef NWK_ENABLE_SECURITY
/*****************************************************************************
*****************************************************************************/
void nwkRxDecryptConf(NwkFrame_t *frame, bool status)
{
  if (status)
    frame->state = NWK_RX_STATE_INDICATE;
  else
    frame->state = NWK_RX_STATE_FINISH;
}
#endif

/*****************************************************************************
*****************************************************************************/
static void nwkRxDuplicateRejectionTimerHandler(SYS_Timer_t *timer)
{
  bool restart = false;
  uint8_t i = 0;

  for (i = 0; i < NWK_DUPLICATE_REJECTION_TABLE_SIZE; i++)
  {
    if (nwkRxDuplicateRejectionTable[i].ttl)
    {
      nwkRxDuplicateRejectionTable[i].ttl--;
      restart = true;
    }
  }

  if (restart)
    SYS_TimerStart(timer);
}

/*****************************************************************************
*****************************************************************************/
static bool nwkRxRejectDuplicate(NwkFrameHeader_t *header)
{
  int8_t free = -1;
  uint8_t i = 0;

  for (i = 0; i < NWK_DUPLICATE_REJECTION_TABLE_SIZE; i++)
  {
    if (nwkRxDuplicateRejectionTable[i].ttl)
    {
      if (header->nwkSrcAddr == nwkRxDuplicateRejectionTable[i].src)
      {
        int8_t diff = (int8_t)header->nwkSeq - nwkRxDuplicateRejectionTable[i].seq;

        if (diff > 0)
        {
          nwkRxDuplicateRejectionTable[i].seq = header->nwkSeq;
          nwkRxDuplicateRejectionTable[i].ttl = DUPLICATE_REJECTION_TTL;
          return false;
        }
        else
        {
#ifdef NWK_ENABLE_ROUTING
          if (nwkIb.addr == header->macDstAddr)
            nwkRouteRemove(header->nwkDstAddr);
#endif
          return true;
        }
      }
    }
    else // ttl == 0
    {
      free = i;
    }
  }

  if (-1 == free)
    return true;

  nwkRxDuplicateRejectionTable[free].src = header->nwkSrcAddr;
  nwkRxDuplicateRejectionTable[free].seq = header->nwkSeq;
  nwkRxDuplicateRejectionTable[free].ttl = DUPLICATE_REJECTION_TTL;

  SYS_TimerStart(&nwkRxDuplicateRejectionTimer);

  return false;
}

/*****************************************************************************
*****************************************************************************/
static bool nwkRxSeriveDataInd(NWK_DataInd_t *ind)
{
  uint8_t cmd = ind->data[0];

  if (NWK_COMMAND_ACK == cmd)
    nwkTxAckReceived(ind);
#ifdef NWK_ENABLE_ROUTING
  else if (NWK_COMMAND_ROUTE_ERROR == cmd)
    nwkRouteErrorReceived(ind);
#endif
  else
    return false;

  return true;
}

/*****************************************************************************
*****************************************************************************/
static bool nwkRxIndicateFrame(NwkFrame_t *frame)
{
  NwkFrameHeader_t *header = &frame->data.header;
  NWK_DataInd_t ind;

  if (header->nwkDstEndpoint > NWK_MAX_ENDPOINTS_AMOUNT ||
      NULL == nwkIb.endpoint[header->nwkDstEndpoint])
    return false;

  ind.srcAddr = header->nwkSrcAddr;
  ind.srcEndpoint = header->nwkSrcEndpoint;
  ind.dstEndpoint = header->nwkDstEndpoint;
  ind.data = frame->data.payload;
  ind.size = frame->size - sizeof(NwkFrameHeader_t);
  ind.lqi = frame->rx.lqi;
  ind.rssi = frame->rx.rssi;

  ind.options  = (header->nwkFcf.ackRequest) ? NWK_IND_OPT_ACK_REQUESTED : 0;
  ind.options |= (header->nwkFcf.securityEnabled) ? NWK_IND_OPT_SECURED : 0;
  ind.options |= (header->nwkFcf.linkLocal) ? NWK_IND_OPT_LINK_LOCAL : 0;
  ind.options |= (0xffff == header->nwkDstAddr) ? NWK_IND_OPT_BROADCAST : 0;
  ind.options |= (header->nwkSrcAddr == header->macSrcAddr) ? NWK_IND_OPT_LOCAL : 0;
  ind.options |= (0xffff == header->macDstPanId) ? NWK_IND_OPT_BROADCAST_PAN_ID : 0;

  return nwkIb.endpoint[header->nwkDstEndpoint](&ind);
}

/*****************************************************************************
*****************************************************************************/
static void nwkRxHandleReceivedFrame(NwkFrame_t *frame)
{
  NwkFrameHeader_t *header = &frame->data.header;

  frame->state = NWK_RX_STATE_FINISH;

  if ((0xffff == header->nwkDstAddr && header->nwkFcf.ackRequest) ||
      (nwkIb.addr == header->nwkSrcAddr))
    return;

#ifndef NWK_ENABLE_SECURITY
  if (header->nwkFcf.securityEnabled)
    return;
#endif

#ifdef NWK_ENABLE_ROUTING
  nwkRouteFrameReceived(frame);
#endif

  if (nwkRxRejectDuplicate(header))
    return;

  if (0xffff == header->macDstAddr && nwkIb.addr != header->nwkDstAddr &&
      0xffff != header->macDstPanId && 0 == header->nwkFcf.linkLocal)
    nwkTxBroadcastFrame(frame);

  if (nwkIb.addr == header->nwkDstAddr || 0xffff == header->nwkDstAddr)
  {
#ifdef NWK_ENABLE_SECURITY
    if (header->nwkFcf.securityEnabled)
      frame->state = NWK_RX_STATE_DECRYPT;
    else
#endif
      frame->state = NWK_RX_STATE_INDICATE;
  }
#ifdef NWK_ENABLE_ROUTING
  else if (nwkIb.addr == header->macDstAddr && 0xffff != header->macDstPanId)
  {
    frame->state = NWK_RX_STATE_ROUTE;
  }
#endif
}

/*****************************************************************************
*****************************************************************************/
void nwkRxTaskHandler(void)
{
  if (0 == nwkRxActiveFrames)
    return;

  int i = 0;

  for (i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    NwkFrame_t *frame = nwkFrameByIndex(i);

    switch (frame->state)
    {
      case NWK_RX_STATE_RECEIVED:
      {
        nwkRxHandleReceivedFrame(frame);
      } break;

#ifdef NWK_ENABLE_SECURITY
      case NWK_RX_STATE_DECRYPT:
      {
        nwkSecurityProcess(frame, false);
      } break;
#endif

      case NWK_RX_STATE_INDICATE:
      {
        NwkFrameHeader_t *header = &frame->data.header;
        bool ack, forceAck;

        nwkRxAckControl = NWK_ACK_CONTROL_NONE;
        ack = nwkRxIndicateFrame(frame);
        forceAck = (0xffff == header->macDstAddr && nwkIb.addr == header->nwkDstAddr);

        if ((header->nwkFcf.ackRequest && ack) || forceAck)
          nwkRxSendAck(frame);

        frame->state = NWK_RX_STATE_FINISH;
      } break;

#ifdef NWK_ENABLE_ROUTING
      case NWK_RX_STATE_ROUTE:
      {
        nwkRouteFrame(frame);
        --nwkRxActiveFrames;
      } break;
#endif

      case NWK_RX_STATE_FINISH:
      {
        nwkFrameFree(frame);
        --nwkRxActiveFrames;
      } break;
    }
  }
}