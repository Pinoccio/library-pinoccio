/**
 * \file otaServer.c
 *
 * \brief OTA Server implementation
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
 * $Id: otaServer.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "nwk.h"
#include "sysTimer.h"
#include "otaCommon.h"
#include "otaServer.h"

#ifdef APP_ENABLE_OTA

/*- Types ------------------------------------------------------------------*/
typedef union OtaServerCommand_t
{
  OtaStartReqCommand_t   start;
  OtaBlockReqCommand_t   block;
} OtaServerCommand_t;

typedef enum
{
  OTA_SERVER_STATE_IDLE               = 0x00,
  OTA_SERVER_STATE_READY              = 0x01,
  OTA_SERVER_STATE_WAIT_FRAME_SPACING = 0x02,
  OTA_SERVER_STATE_START_REQ_SENT     = 0x07,
  OTA_SERVER_STATE_WAIT_START_RESP    = 0x87,
  OTA_SERVER_STATE_BLOCK_REQ_SENT     = 0x08,
  OTA_SERVER_STATE_WAIT_BLOCK_RESP    = 0x88,
  OTA_SERVER_STATE_FINISH_REQ_SENT    = 0x09,
  OTA_SERVER_STATE_WAIT_FINISH_RESP   = 0x89,
  OTA_SERVER_STATE_WAIT_RESP_MASK     = 0x80,
} OtaServerState_t;

typedef struct OtaServer_t
{
  OtaServerState_t state;
  uint16_t         sessionId;
  uint16_t         client;
  uint32_t         size;
  uint16_t         crc;
  uint8_t          retries;
} OtaServer_t;

/*- Prototypes -------------------------------------------------------------*/
static void otaDataConf(NWK_DataReq_t *req);
static void otaRequestRetry(void);
static void otaFrameSpacingTimerHandler(SYS_Timer_t *timer);
static void otaResponseTimerHandler(SYS_Timer_t *timer);
static bool otaServerDataInd(NWK_DataInd_t *ind);

/*- Variables --------------------------------------------------------------*/
static OtaServer_t         otaServer;
static OtaServerCommand_t  otaCommand;
static NWK_DataReq_t       otaDataReq;
static SYS_Timer_t         otaResponseTimer;
static SYS_Timer_t         otaFrameSpacingTimer;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void OTA_ServerInit(void)
{
  otaServer.state = OTA_SERVER_STATE_IDLE;

  otaDataReq.srcEndpoint = APP_OTA_ENDPOINT;
  otaDataReq.dstEndpoint = APP_OTA_ENDPOINT;
  otaDataReq.data = (uint8_t *)&otaCommand;
  otaDataReq.confirm = otaDataConf;

  otaResponseTimer.interval = OTA_RESPONSE_TIMEOUT;
  otaResponseTimer.mode = SYS_TIMER_INTERVAL_MODE;
  otaResponseTimer.handler = otaResponseTimerHandler;

  otaFrameSpacingTimer.interval = OTA_FRAME_SPACING;
  otaFrameSpacingTimer.mode = SYS_TIMER_INTERVAL_MODE;
  otaFrameSpacingTimer.handler = otaFrameSpacingTimerHandler;

  NWK_OpenEndpoint(APP_OTA_ENDPOINT, otaServerDataInd);
}

/*************************************************************************//**
*****************************************************************************/
void OTA_ServerStartUpdrade(uint16_t addr, uint32_t size)
{
  SYS_TimerStop(&otaResponseTimer);
  SYS_TimerStop(&otaFrameSpacingTimer);

  otaServer.state = OTA_SERVER_STATE_START_REQ_SENT;
  otaServer.sessionId = rand();
  otaServer.client = addr;
  otaServer.size = size;
  otaServer.crc = 0;
  otaServer.retries = OTA_MAX_RETRIES;

  otaDataReq.dstAddr = addr;
  otaDataReq.size = sizeof(OtaStartReqCommand_t);
  otaDataReq.options = NWK_OPT_ACK_REQUEST;

  otaCommand.start.commandId = OTA_START_REQ_COMMAND_ID;
  otaCommand.start.sessionId = otaServer.sessionId;
  otaCommand.start.size = size;

  NWK_DataReq(&otaDataReq);
}

/*************************************************************************//**
*****************************************************************************/
void OTA_ServerSendBlock(uint8_t *data)
{
  uint8_t size;

  if (OTA_SERVER_STATE_READY != otaServer.state)
    return;

  if (otaServer.size > OTA_MAX_BLOCK_SIZE)
    size = OTA_MAX_BLOCK_SIZE;
  else
    size = otaServer.size;

  otaServer.state = OTA_SERVER_STATE_BLOCK_REQ_SENT;
  otaServer.size -= size;

  for (uint8_t i = 0; i < size; i++)
    otaServer.crc = otaCrcUpdateCcitt(otaServer.crc, data[i]);

  otaDataReq.dstAddr = otaServer.client;
  otaDataReq.size = sizeof(OtaBlockReqHeader_t) + size;
  otaDataReq.options = 0;

  otaCommand.block.commandId = OTA_BLOCK_REQ_COMMAND_ID;
  otaCommand.block.sessionId = otaServer.sessionId;
  otaCommand.block.crc = otaServer.crc;
  otaCommand.block.size = size;

  memcpy(otaCommand.block.data, data, size);

  NWK_DataReq(&otaDataReq);
}

/*************************************************************************//**
*****************************************************************************/
static void otaDataConf(NWK_DataReq_t *req)
{
  if (NWK_SUCCESS_STATUS == req->status)
  {
    otaServer.state |= OTA_SERVER_STATE_WAIT_RESP_MASK;
    SYS_TimerStart(&otaResponseTimer);
  }
  else
  {
    otaRequestRetry();
  }
}

/*************************************************************************//**
*****************************************************************************/
static void otaRequestRetry(void)
{
  if (0 == --otaServer.retries)
  {
    otaServer.state = OTA_SERVER_STATE_IDLE;
    OTA_ServerNotification(OTA_NO_RESPONSE_STATUS);
  }
  else
  {
    otaServer.state &= ~OTA_SERVER_STATE_WAIT_RESP_MASK;
    NWK_DataReq(&otaDataReq);
  }
}

/*************************************************************************//**
*****************************************************************************/
static void otaResponseTimerHandler(SYS_Timer_t *timer)
{
  otaRequestRetry();
  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static void otaFrameSpacingTimerHandler(SYS_Timer_t *timer)
{
  otaServer.state = OTA_SERVER_STATE_READY;
  OTA_ServerNotification(OTA_CLIENT_READY_STATUS);

  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static bool otaHandleStartResp(NWK_DataInd_t *ind)
{
  OtaStartRespCommand_t *resp = (OtaStartRespCommand_t *)ind->data;

  if (ind->size != sizeof(OtaStartRespCommand_t))
    return false;

  if (OTA_SERVER_STATE_WAIT_START_RESP != otaServer.state ||
      otaServer.sessionId != resp->sessionId)
    return true;

  otaServer.retries = OTA_MAX_RETRIES;
  SYS_TimerStop(&otaResponseTimer);

  if (OTA_SUCCESS_STATUS == resp->status)
  {
    otaServer.state = OTA_SERVER_STATE_READY;
    OTA_ServerNotification(OTA_CLIENT_READY_STATUS);
  }
  else
  {
    otaServer.state = OTA_SERVER_STATE_IDLE;
    OTA_ServerNotification((OTA_Status_t)resp->status);
  }

  return true;
}

/*************************************************************************//**
*****************************************************************************/
static bool otaHandleBlockResp(NWK_DataInd_t *ind)
{
  OtaBlockRespCommand_t *resp = (OtaBlockRespCommand_t *)ind->data;

  if (ind->size != sizeof(OtaBlockRespCommand_t))
    return false;

  if (OTA_SERVER_STATE_WAIT_BLOCK_RESP != otaServer.state ||
      otaServer.sessionId != resp->sessionId)
    return true;

  otaServer.retries = OTA_MAX_RETRIES;
  SYS_TimerStop(&otaResponseTimer);

  if (OTA_SUCCESS_STATUS == resp->status)
  {
    otaServer.state = OTA_SERVER_STATE_WAIT_FRAME_SPACING;
    SYS_TimerStart(&otaFrameSpacingTimer);
  }
  else
  {
    otaServer.state = OTA_SERVER_STATE_IDLE;
    OTA_ServerNotification((OTA_Status_t)resp->status);
  }

  return true;
}

/*************************************************************************//**
*****************************************************************************/
static bool otaServerDataInd(NWK_DataInd_t *ind)
{
  OtaCommandHeader_t *header = (OtaCommandHeader_t *)ind->data;

  if (OTA_START_RESP_COMMAND_ID == header->commandId)
    return otaHandleStartResp(ind);

  else if (OTA_BLOCK_RESP_COMMAND_ID == header->commandId)
    return otaHandleBlockResp(ind);

  return false;
}

/*************************************************************************//**
*****************************************************************************/
void OTA_ServerTaskHandler(void)
{
  // empty
}

#endif // APP_ENABLE_OTA
