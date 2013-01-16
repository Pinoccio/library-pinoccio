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
#define NWK_TX_ACK_WAIT_TIMER_INTERVAL    50 // ms

/*****************************************************************************
*****************************************************************************/
enum
{
  NWK_TX_STATE_ENCRYPT   = 0x10,
  NWK_TX_STATE_SEND      = 0x11,
  NWK_TX_STATE_WAIT_CONF = 0x12,
  NWK_TX_STATE_SENT      = 0x13,
  NWK_TX_STATE_WAIT_ACK  = 0x14,
  NWK_TX_STATE_CONFIRM   = 0x15,
};

/*****************************************************************************
*****************************************************************************/
static void nwkTxBroadcastConf(NwkFrame_t *frame);
static void nwkTxAckWaitTimerHandler(SYS_Timer_t *timer);

/*****************************************************************************
*****************************************************************************/
static NwkFrame_t *nwkTxPhyActiveFrame;
static uint8_t nwkTxActiveFrames;
static SYS_Timer_t nwkTxAckWaitTimer;

/*****************************************************************************
*****************************************************************************/
void nwkTxInit(void)
{
  nwkTxPhyActiveFrame = NULL;
  nwkTxActiveFrames = 0;

  nwkTxAckWaitTimer.interval = NWK_TX_ACK_WAIT_TIMER_INTERVAL;
  nwkTxAckWaitTimer.mode = SYS_TIMER_INTERVAL_MODE;
  nwkTxAckWaitTimer.handler = nwkTxAckWaitTimerHandler;
}

/*****************************************************************************
*****************************************************************************/
void nwkTxFrame(NwkFrame_t *frame)
{
  NwkFrameHeader_t *header = &frame->data.header;

  if (frame->tx.control & NWK_TX_CONTROL_ROUTING)
  {
    frame->state = NWK_TX_STATE_SEND;
  }
  else
  {
#ifdef NWK_ENABLE_SECURITY
    if (frame->data.header.nwkFcf.securityEnabled)
      frame->state = NWK_TX_STATE_ENCRYPT;
    else
#endif
      frame->state = NWK_TX_STATE_SEND;
  }

  frame->tx.status = NWK_SUCCESS_STATUS;

  if (frame->tx.control & NWK_TX_CONTROL_BROADCAST_PAN_ID)
    frame->data.header.macDstPanId = 0xffff;
  else
    frame->data.header.macDstPanId = nwkIb.panId;

#ifdef NWK_ENABLE_ROUTING
  header->macDstAddr = nwkRouteNextHop(header->nwkDstAddr);
#else
  header->macDstAddr = header->nwkDstAddr;
#endif
  header->macSrcAddr = nwkIb.addr;
  header->macSeq = ++nwkIb.macSeqNum;

  if (0xffff == header->macDstAddr)
    header->macFcf = 0x8841;
  else
    header->macFcf = 0x8861;

  ++nwkTxActiveFrames;
}

/*****************************************************************************
*****************************************************************************/
void nwkTxBroadcastFrame(NwkFrame_t *frame)
{
  NwkFrame_t *newFrame;

  if (NULL == (newFrame = nwkFrameAlloc(frame->size - sizeof(NwkFrameHeader_t))))
    return;

  newFrame->tx.confirm = nwkTxBroadcastConf;
  memcpy((uint8_t *)&newFrame->data, (uint8_t *)&frame->data, frame->size);

  newFrame->state = NWK_TX_STATE_SEND;
  newFrame->tx.status = NWK_SUCCESS_STATUS;

  newFrame->data.header.macFcf = 0x8841;
  newFrame->data.header.macDstAddr = 0xffff;
  newFrame->data.header.macDstPanId = nwkIb.panId;
  newFrame->data.header.macSrcAddr = nwkIb.addr;
  newFrame->data.header.macSeq = ++nwkIb.macSeqNum;

  ++nwkTxActiveFrames;
}

/*****************************************************************************
*****************************************************************************/
static void nwkTxBroadcastConf(NwkFrame_t *frame)
{
  nwkFrameFree(frame);
}

/*****************************************************************************
*****************************************************************************/
void nwkTxAckReceived(NWK_DataInd_t *ind)
{
  NwkAckCommand_t *command = (NwkAckCommand_t *)ind->data;

  if (0 == nwkTxActiveFrames)
    return;
  int i = 0;

  for (i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    NwkFrame_t *frame = nwkFrameByIndex(i);

    if (NWK_TX_STATE_WAIT_ACK == frame->state && frame->data.header.nwkSeq == command->seq)
    {
      frame->state = NWK_TX_STATE_CONFIRM;
      frame->tx.control = command->control;
      return;
    }
  }
}

/*****************************************************************************
*****************************************************************************/
bool nwkTxBusy(void)
{
  return nwkTxActiveFrames > 0;
}

/*****************************************************************************
*****************************************************************************/
static void nwkTxAckWaitTimerHandler(SYS_Timer_t *timer)
{
  if (0 == nwkTxActiveFrames)
    return;
  int i = 0;

  for (i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    NwkFrame_t *frame = nwkFrameByIndex(i);

    if (NWK_TX_STATE_WAIT_ACK == frame->state && 0 == --frame->tx.timeout)
    {
      frame->state = NWK_TX_STATE_CONFIRM;
      frame->tx.status = NWK_NO_ACK_STATUS;
    }
  }

  SYS_TimerStart(timer);
}

#ifdef NWK_ENABLE_SECURITY
/*****************************************************************************
*****************************************************************************/
void nwkTxEncryptConf(NwkFrame_t *frame)
{
  frame->state = NWK_TX_STATE_SEND;
}
#endif

/*****************************************************************************
*****************************************************************************/
static uint8_t convertPhyStatus(uint8_t status)
{
  if (TRAC_STATUS_SUCCESS == status ||
      TRAC_STATUS_SUCCESS_DATA_PENDING == status ||
      TRAC_STATUS_SUCCESS_WAIT_FOR_ACK == status)
    return NWK_SUCCESS_STATUS;

  else if (TRAC_STATUS_CHANNEL_ACCESS_FAILURE == status)
    return NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS;

  else if (TRAC_STATUS_NO_ACK == status)
    return NWK_PHY_NO_ACK_STATUS;

  else
    return NWK_ERROR_STATUS;
}

/*****************************************************************************
*****************************************************************************/
void PHY_DataConf(uint8_t status)
{
  nwkTxPhyActiveFrame->tx.status = convertPhyStatus(status);
  nwkTxPhyActiveFrame->state = NWK_TX_STATE_SENT;
  nwkTxPhyActiveFrame = NULL;
}

/*****************************************************************************
*****************************************************************************/
void nwkTxTaskHandler(void)
{
  if (0 == nwkTxActiveFrames)
    return;
  int i = 0;

  for (i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    NwkFrame_t *frame = nwkFrameByIndex(i);

    switch (frame->state)
    {
#ifdef NWK_ENABLE_SECURITY
      case NWK_TX_STATE_ENCRYPT:
      {
        nwkSecurityProcess(frame, true);
      } break;
#endif

      case NWK_TX_STATE_SEND:
      {
        if (!PHY_Busy())
        {
          nwkTxPhyActiveFrame = frame;
          frame->state = NWK_TX_STATE_WAIT_CONF;
          PHY_DataReq((uint8_t *)&frame->data, frame->size);
        }
      } break;

      case NWK_TX_STATE_WAIT_CONF:
        break;

      case NWK_TX_STATE_SENT:
      {
        if (NWK_SUCCESS_STATUS == frame->tx.status)
        {
          if (frame->data.header.nwkSrcAddr == nwkIb.addr &&
              frame->data.header.nwkFcf.ackRequest)
          {
            frame->state = NWK_TX_STATE_WAIT_ACK;
            frame->tx.timeout = NWK_ACK_WAIT_TIME / NWK_TX_ACK_WAIT_TIMER_INTERVAL + 1;
            SYS_TimerStart(&nwkTxAckWaitTimer);
          }
          else
          {
            frame->state = NWK_TX_STATE_CONFIRM;
          }
        }
        else
        {
          frame->state = NWK_TX_STATE_CONFIRM;
  }
      } break;

      case NWK_TX_STATE_WAIT_ACK:
        break;

      case NWK_TX_STATE_CONFIRM:
      {
#ifdef NWK_ENABLE_ROUTING
        nwkRouteFrameSent(frame);
#endif
        frame->tx.confirm(frame);
        --nwkTxActiveFrames;
      } break;

      default:
        break;
    };
  }
}