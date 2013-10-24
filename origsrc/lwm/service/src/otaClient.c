/**
 * \file otaClient.c
 *
 * \brief OTA Client implementation
 *
 * Copyright (C) 2012-2013, Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 * $Id: otaClient.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "nwk.h"
#include "sysTimer.h"
#include "otaCommon.h"
#include "otaClient.h"

#ifdef APP_ENABLE_OTA

/*- Types ------------------------------------------------------------------*/
typedef union OtaClientCommand_t
{
  OtaStartRespCommand_t   start;
  OtaBlockRespCommand_t   block;
} OtaClientCommand_t;

typedef enum
{
  OTA_CLIENT_STATE_IDLE,
  OTA_CLIENT_STATE_START_RESP_SENT,
  OTA_CLIENT_STATE_READY,
  OTA_CLIENT_STATE_WAIT_BLOCK_CONF,
  OTA_CLIENT_STATE_BLOCK_RESP_SENT,
} OtaClientState_t;

typedef struct OtaClient_t
{
  OtaClientState_t state;
  uint16_t         sessionId;
  uint16_t         server;
  uint8_t          status;
  uint32_t         size;
  uint16_t         crc;
  uint8_t          retries;
} OtaClient_t;

/*- Prototypes -------------------------------------------------------------*/
static void otaDataConf(NWK_DataReq_t *req);
static void otaClientSendResponse(OTA_Status_t status);
static void otaRequestTimerHandler(SYS_Timer_t *timer);
static bool otaClientDataInd(NWK_DataInd_t *ind);

/*- Variables --------------------------------------------------------------*/
static OtaClient_t         otaClient;
static OtaClientCommand_t  otaCommand;
static NWK_DataReq_t       otaDataReq;
static SYS_Timer_t         otaRequestTimer;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void OTA_ClientInit(void)
{
  otaClient.state = OTA_CLIENT_STATE_IDLE;

  otaDataReq.srcEndpoint = APP_OTA_ENDPOINT;
  otaDataReq.dstEndpoint = APP_OTA_ENDPOINT;
  otaDataReq.data = (uint8_t *)&otaCommand;
  otaDataReq.confirm = otaDataConf;

  otaRequestTimer.interval = OTA_RESPONSE_TIMEOUT * OTA_MAX_RETRIES;
  otaRequestTimer.mode = SYS_TIMER_INTERVAL_MODE;
  otaRequestTimer.handler = otaRequestTimerHandler;

  NWK_OpenEndpoint(APP_OTA_ENDPOINT, otaClientDataInd);
}

/*************************************************************************//**
*****************************************************************************/
void OTA_ClientBlockConfirm(OTA_Status_t status)
{
  if (OTA_SUCCESS_STATUS == status && 0 == otaClient.size)
    otaClientSendResponse(OTA_UPGRADE_COMPLETED_STATUS);
  else
    otaClientSendResponse(status);
}

/*************************************************************************//**
*****************************************************************************/
static void otaDataConf(NWK_DataReq_t *req)
{
  if (NWK_SUCCESS_STATUS == req->status)
  {
    otaClient.retries = OTA_MAX_RETRIES;

    if (OTA_CLIENT_STATE_START_RESP_SENT == otaClient.state)
    {
      otaClient.state = OTA_CLIENT_STATE_READY;
      OTA_ClientNotification(OTA_UPGRADE_STARTED_STATUS);
      SYS_TimerStart(&otaRequestTimer);
    }

    else if (OTA_CLIENT_STATE_BLOCK_RESP_SENT == otaClient.state)
    {
      if (OTA_SUCCESS_STATUS == otaCommand.block.status)
      {
        otaClient.state = OTA_CLIENT_STATE_READY;
        SYS_TimerStart(&otaRequestTimer);
      }
      else if (otaCommand.block.status & OTA_APPLICATION_STATUS_MASK)
      {
        otaClient.state = OTA_CLIENT_STATE_IDLE;
      }
      else
      {
        otaClient.state = OTA_CLIENT_STATE_IDLE;
        OTA_ClientNotification((OTA_Status_t)otaCommand.block.status);
      }
    }
  }
  else
  {
    if (0 == --otaClient.retries)
    {
      otaClient.state = OTA_CLIENT_STATE_IDLE;
      OTA_ClientNotification(OTA_NETWORK_ERROR_STATUS);
    }
    else
    {
      NWK_DataReq(&otaDataReq);
    }
  }
}

/*************************************************************************//**
*****************************************************************************/
static void otaRequestTimerHandler(SYS_Timer_t *timer)
{
  otaClient.state = OTA_CLIENT_STATE_IDLE;
  OTA_ClientNotification(OTA_SESSION_TIMEOUT_STATUS);
  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static void otaClientSendResponse(OTA_Status_t status)
{
  otaClient.state = OTA_CLIENT_STATE_BLOCK_RESP_SENT;

  otaDataReq.dstAddr = otaClient.server;
  otaDataReq.size = sizeof(OtaBlockRespCommand_t);
  otaDataReq.options = 0;

  otaCommand.block.commandId = OTA_BLOCK_RESP_COMMAND_ID;
  otaCommand.block.sessionId = otaClient.sessionId;
  otaCommand.block.status = status;

  NWK_DataReq(&otaDataReq);
}

/*************************************************************************//**
*****************************************************************************/
static bool otaHandleStartReq(NWK_DataInd_t *ind)
{
  OtaStartReqCommand_t *req = (OtaStartReqCommand_t *)ind->data;

  if (ind->size != sizeof(OtaStartReqCommand_t))
    return false;

  if (OTA_CLIENT_STATE_IDLE != otaClient.state)
    return true;

  otaClient.state = OTA_CLIENT_STATE_START_RESP_SENT;
  otaClient.status = OTA_SUCCESS_STATUS;
  otaClient.sessionId = req->sessionId;
  otaClient.server = ind->srcAddr;
  otaClient.size = req->size;
  otaClient.crc = 0;
  otaClient.retries = OTA_MAX_RETRIES;

  otaDataReq.dstAddr = otaClient.server;
  otaDataReq.size = sizeof(OtaStartRespCommand_t);
  otaDataReq.options = NWK_OPT_ACK_REQUEST;

  otaCommand.start.commandId = OTA_START_RESP_COMMAND_ID;
  otaCommand.start.sessionId = otaClient.sessionId;
  otaCommand.start.status = OTA_SUCCESS_STATUS;

  NWK_DataReq(&otaDataReq);

  return true;
}

/*************************************************************************//**
*****************************************************************************/
static bool otaHandleBlockReq(NWK_DataInd_t *ind)
{
  OtaBlockReqCommand_t *block = (OtaBlockReqCommand_t *)ind->data;

  if (ind->size < sizeof(OtaBlockReqHeader_t) ||
      ind->size != sizeof(OtaBlockReqHeader_t) + block->size)
    return false;

  if (OTA_CLIENT_STATE_READY != otaClient.state ||
      otaClient.sessionId != block->sessionId)
    return true;

  SYS_TimerStop(&otaRequestTimer);

  otaClient.size -= block->size;

  for (uint8_t i = 0; i < block->size; i++)
    otaClient.crc = otaCrcUpdateCcitt(otaClient.crc, block->data[i]);

  if (otaClient.crc == block->crc)
  {
    otaClient.state = OTA_CLIENT_STATE_WAIT_BLOCK_CONF;
    OTA_ClientBlockIndication(block->size, block->data);
  }
  else
  {
    otaClientSendResponse(OTA_CRC_ERROR_STATUS);
  }

  return true;
}

/*************************************************************************//**
*****************************************************************************/
static bool otaClientDataInd(NWK_DataInd_t *ind)
{
  OtaCommandHeader_t *header = (OtaCommandHeader_t *)ind->data;

  if (OTA_START_REQ_COMMAND_ID == header->commandId)
    return otaHandleStartReq(ind);

  else if (OTA_BLOCK_REQ_COMMAND_ID == header->commandId)
    return otaHandleBlockReq(ind);

  return false;
}

/*************************************************************************//**
*****************************************************************************/
void OTA_ClientTaskHandler(void)
{
  // empty
}

#endif // APP_ENABLE_OTA
