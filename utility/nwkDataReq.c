/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sysConfig.h"
#include "nwk.h"
#include "nwkTx.h"
#include "nwkFrame.h"
#include "nwkGroup.h"
#include "nwkDataReq.h"

/*- Types ------------------------------------------------------------------*/
enum
{
  NWK_DATA_REQ_STATE_INITIAL,
  NWK_DATA_REQ_STATE_WAIT_CONF,
  NWK_DATA_REQ_STATE_CONFIRM,
};

/*- Prototypes -------------------------------------------------------------*/
static void nwkDataReqTxConf(NwkFrame_t *frame);

/*- Variables --------------------------------------------------------------*/
static NWK_DataReq_t *nwkDataReqQueue;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
  @brief Initializes the Data Request module
*****************************************************************************/
void nwkDataReqInit(void)
{
  nwkDataReqQueue = NULL;
}

/*************************************************************************//**
  @brief Adds request @a req to the queue of outgoing requests
  @param[in] req Pointer to the request parameters
*****************************************************************************/
void NWK_DataReq(NWK_DataReq_t *req)
{
  req->state = NWK_DATA_REQ_STATE_INITIAL;
  req->status = NWK_SUCCESS_STATUS;
  req->frame = NULL;

  if (NULL == nwkDataReqQueue)
  {
    req->next = NULL;
    nwkDataReqQueue = req;
  }
  else
  {
    req->next = nwkDataReqQueue;
    nwkDataReqQueue = req;
  }
}

/*************************************************************************//**
  @brief Prepares and send outgoing frame based on the request @a req parameters
  @param[in] req Pointer to the request parameters
*****************************************************************************/
static void nwkDataReqSendFrame(NWK_DataReq_t *req)
{
  NwkFrame_t *frame;

  if (NULL == (frame = nwkFrameAlloc()))
  {
    req->state = NWK_DATA_REQ_STATE_CONFIRM;
    req->status = NWK_OUT_OF_MEMORY_STATUS;
    return;
  }

  req->frame = frame;
  req->state = NWK_DATA_REQ_STATE_WAIT_CONF;

  frame->tx.confirm = nwkDataReqTxConf;
  frame->tx.control = req->options & NWK_OPT_BROADCAST_PAN_ID ? NWK_TX_CONTROL_BROADCAST_PAN_ID : 0;

  frame->header.nwkFcf.ackRequest = req->options & NWK_OPT_ACK_REQUEST ? 1 : 0;
  frame->header.nwkFcf.linkLocal = req->options & NWK_OPT_LINK_LOCAL ? 1 : 0;

#ifdef NWK_ENABLE_SECURITY
  frame->header.nwkFcf.security = req->options & NWK_OPT_ENABLE_SECURITY ? 1 : 0;
#endif

#ifdef NWK_ENABLE_MULTICAST
  frame->header.nwkFcf.multicast = req->options & NWK_OPT_MULTICAST ? 1 : 0;

  if (frame->header.nwkFcf.multicast)
  {
    NwkFrameMulticastHeader_t *mcHeader = (NwkFrameMulticastHeader_t *)frame->payload;

    frame->header.nwkFcf.linkLocal = 1;

    mcHeader->memberRadius = req->memberRadius;
    mcHeader->maxMemberRadius = req->memberRadius;
    mcHeader->nonMemberRadius = req->nonMemberRadius;
    mcHeader->maxNonMemberRadius = req->nonMemberRadius;

    frame->payload += sizeof(NwkFrameMulticastHeader_t);
    frame->size += sizeof(NwkFrameMulticastHeader_t);
  }
#endif

  frame->header.nwkSeq = ++nwkIb.nwkSeqNum;
  frame->header.nwkSrcAddr = nwkIb.addr;
  frame->header.nwkDstAddr = req->dstAddr;
  frame->header.nwkSrcEndpoint = req->srcEndpoint;
  frame->header.nwkDstEndpoint = req->dstEndpoint;

  memcpy(frame->payload, req->data, req->size);
  frame->size += req->size;

  nwkTxFrame(frame);
}

/*************************************************************************//**
  @brief Frame transmission confirmation handler
  @param[in] frame Pointer to the sent frame
*****************************************************************************/
static void nwkDataReqTxConf(NwkFrame_t *frame)
{
  NWK_DataReq_t *req;
  for (req = nwkDataReqQueue; req; req = req->next)
  {
    if (req->frame == frame)
    {
      req->status = frame->tx.status;
      req->control = frame->tx.control;
      req->state = NWK_DATA_REQ_STATE_CONFIRM;
      break;
    }
  }

  nwkFrameFree(frame);
}

/*************************************************************************//**
  @brief Confirms request @req to the application and remove it from the queue
  @param[in] req Pointer to the request parameters
*****************************************************************************/
static void nwkDataReqConfirm(NWK_DataReq_t *req)
{
  if (nwkDataReqQueue == req)
  {
    nwkDataReqQueue = nwkDataReqQueue->next;
  }
  else
  {
    NWK_DataReq_t *prev = nwkDataReqQueue;
    while (prev->next != req)
      prev = prev->next;
    prev->next = ((NWK_DataReq_t *)prev->next)->next;
  }

  req->confirm(req);
}

/*************************************************************************//**
  @brief Data Request module task handler
*****************************************************************************/
void nwkDataReqTaskHandler(void)
{
  NWK_DataReq_t *req;
  for (req = nwkDataReqQueue; req; req = req->next)
  {
    switch (req->state)
    {
      case NWK_DATA_REQ_STATE_INITIAL:
      {
        nwkDataReqSendFrame(req);
        return;
      } break;

      case NWK_DATA_REQ_STATE_WAIT_CONF:
        break;

      case NWK_DATA_REQ_STATE_CONFIRM:
      {
        nwkDataReqConfirm(req);
        return;
      } break;

      default:
        break;
    };
  }
}
