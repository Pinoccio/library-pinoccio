/**
 * \file OTAServerDemo.c
 *
 * \brief OTAServerDemo application implementation
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
 * $Id: OTAServerDemo.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halUart.h"
#include "otaServer.h"

/*- Definitions ------------------------------------------------------------*/
#define APP_UART_TIMEOUT         100    // ms
#define APP_SESSION_TIMEOUT      10000  // ms

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t
{
  APP_STATE_INITIAL,
  APP_STATE_WAIT_COMMAND,
  APP_STATE_COMMAND_RECEIVED,
  APP_STATE_WAIT_NOTIFICATION,
} AppState_t;

typedef enum AppUartState_t
{
  APP_UART_STATE_IDLE,
  APP_UART_STATE_READ_SIZE,
  APP_UART_STATE_READ_DATA,
  APP_UART_STATE_READ_CSUM,
  APP_UART_STATE_READ_FOOTER,
  APP_UART_STATE_COMMAND_RECEIVED,
  APP_UART_STATE_ERROR,
} AppUartState_t;

typedef enum AppUartStatus_t
{
  APP_UART_STATUS_CONFIRMATION            = 0x80,
  APP_UART_STATUS_PASSED                  = 0x81,
  APP_UART_STATUS_UPGRADE_IN_PROGRESS     = 0x82,
  APP_UART_STATUS_NO_UPGRADE_IN_PROGRESS  = 0x83,
  APP_UART_STATUS_UNKNOWN_COMMAND         = 0x84,
  APP_UART_STATUS_MALFORMED_REQUEST       = 0x85,
  APP_UART_STATUS_MALFORMED_COMMAND       = 0x86,
  APP_UART_STATUS_SESSION_TIMEOUT         = 0x87,
} AppUartStatus_t;

typedef enum AppUartCommand_t
{
  APP_UART_COMMAND_COMM_CHECK     = 0x01,
  APP_UART_COMMAND_START_REQUEST  = 0x02,
  APP_UART_COMMAND_BLOCK_REQUEST  = 0x03,
  APP_UART_COMMAND_NOTIFICATION   = 0x04,
} AppUartCommand_t;

typedef struct AppUartCommandStartReq_t
{
  uint8_t    commandId;
  uint16_t   addr;
  uint16_t   panId;
  uint8_t    channel;
  uint16_t   clientAddr;
  uint32_t   dataSize;
} AppUartCommandStartReq_t;

/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appUartTimeoutTimer;
static SYS_Timer_t appSessionTimeoutTimer;

static AppUartState_t appUartState = APP_UART_STATE_IDLE;
static bool appUpgradeInProgress = false;
static uint8_t appUartBuffer[OTA_MAX_BLOCK_SIZE + 10];
static uint8_t appUartSize;
static uint8_t appUartPtr;
static uint16_t appUartCsum;
static uint8_t appUartCsumByte;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static void appUartSendNotification(uint8_t status)
{
  uint16_t csum = 0x1234 + APP_UART_COMMAND_NOTIFICATION + status;

  HAL_UartWriteByte(0x55);
  HAL_UartWriteByte(2);
  HAL_UartWriteByte(APP_UART_COMMAND_NOTIFICATION);
  HAL_UartWriteByte(status);
  HAL_UartWriteByte(csum & 0xff);
  HAL_UartWriteByte((csum >> 8) & 0xff);
  HAL_UartWriteByte(0xaa);
}

/*************************************************************************//**
*****************************************************************************/
static void appUartStateMachine(uint8_t byte)
{
  switch (appUartState)
  {
    case APP_UART_STATE_IDLE:
    {
      appUartSize = 0;
      appUartPtr = 0;
      appUartCsum = 0x1234;
      appUartCsumByte = 0;

      if (0x55 == byte)
        appUartState = APP_UART_STATE_READ_SIZE;
      else
        appUartState = APP_UART_STATE_ERROR;
    } break;

    case APP_UART_STATE_READ_SIZE:
    {
      appUartSize = byte;

      if (1 <= appUartSize || appUartSize <= sizeof(appUartBuffer))
        appUartState = APP_UART_STATE_READ_DATA;
      else
        appUartState = APP_UART_STATE_ERROR;
    } break;

    case APP_UART_STATE_READ_DATA:
    {
      appUartBuffer[appUartPtr++] = byte;
      appUartCsum += byte;

      if (appUartPtr == appUartSize)
        appUartState = APP_UART_STATE_READ_CSUM;
      else if (appUartPtr == sizeof(appUartBuffer))
        appUartState = APP_UART_STATE_ERROR;
    } break;

    case APP_UART_STATE_READ_CSUM:
    {
      if (0 == appUartCsumByte)
      {
        if (byte == (appUartCsum & 0xff))
          appUartCsumByte++;
        else
          appUartState = APP_UART_STATE_ERROR;
      }
      else
      {
        if (byte == ((appUartCsum >> 8) & 0xff))
          appUartState = APP_UART_STATE_READ_FOOTER;
        else
          appUartState = APP_UART_STATE_ERROR;
      }
    } break;

    case APP_UART_STATE_READ_FOOTER:
    {
      if (0xaa == byte)
      {
        appUartState = APP_UART_STATE_COMMAND_RECEIVED;
        appState = APP_STATE_COMMAND_RECEIVED;
        appUartSendNotification(APP_UART_STATUS_CONFIRMATION);
      }
      else
        appUartState = APP_UART_STATE_ERROR;
    } break;

    case APP_UART_STATE_COMMAND_RECEIVED:
    case APP_UART_STATE_ERROR:
      break;
  }
}

/*************************************************************************//**
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)
{
  for (uint16_t i = 0; i < bytes; i++)
  {
    uint8_t byte = HAL_UartReadByte();
    appUartStateMachine(byte);
  }

  SYS_TimerStop(&appUartTimeoutTimer);
  SYS_TimerStart(&appUartTimeoutTimer);
}

/*************************************************************************//**
*****************************************************************************/
static void appFinishCommandProcessing(uint8_t status)
{
  appState = APP_STATE_WAIT_COMMAND;
  appUartState = APP_UART_STATE_IDLE;
  appUartSendNotification(status);
}

/*************************************************************************//**
*****************************************************************************/
static void appUartTimeoutTimerHandler(SYS_Timer_t *timer)
{
  if (APP_UART_STATE_COMMAND_RECEIVED == appUartState)
    return;

  if (APP_UART_STATE_ERROR == appUartState)
    appFinishCommandProcessing(APP_UART_STATUS_MALFORMED_REQUEST);

  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static void appSessionTimeoutTimerHandler(SYS_Timer_t *timer)
{
  appFinishCommandProcessing(APP_UART_STATUS_SESSION_TIMEOUT);
  appUpgradeInProgress = false;
  PHY_SetRxState(false);

  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static void appInit(void)
{
#ifdef PLATFORM_RCB128RFA1
  // Enable RCB_BB RS232 level converter
  DDRD = (1 << 4) | (1 << 6) | (1 << 7);
  PORTD = (0 << 4) | (1 << 6) | (1 << 7);
#endif

  appUartTimeoutTimer.interval = APP_UART_TIMEOUT;
  appUartTimeoutTimer.mode = SYS_TIMER_PERIODIC_MODE;
  appUartTimeoutTimer.handler = appUartTimeoutTimerHandler;
  SYS_TimerStart(&appUartTimeoutTimer);

  appSessionTimeoutTimer.interval = APP_SESSION_TIMEOUT;
  appSessionTimeoutTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appSessionTimeoutTimer.handler = appSessionTimeoutTimerHandler;

  appState = APP_STATE_WAIT_COMMAND;
}

/*************************************************************************//**
*****************************************************************************/
static void appCommandReceived(void)
{
  uint8_t command = appUartBuffer[0];

  if (APP_UART_COMMAND_COMM_CHECK == command)
  {
    appFinishCommandProcessing(APP_UART_STATUS_PASSED);
  }

  else if (APP_UART_COMMAND_START_REQUEST == command)
  {
    if (sizeof(AppUartCommandStartReq_t) != appUartSize)
    {
      appFinishCommandProcessing(APP_UART_STATUS_MALFORMED_COMMAND);
    }
    else if (false == appUpgradeInProgress)
    {
      AppUartCommandStartReq_t *req = (AppUartCommandStartReq_t *)appUartBuffer;

      NWK_SetAddr(req->addr);
      NWK_SetPanId(req->panId);
      PHY_SetChannel(req->channel);
      PHY_SetRxState(true);

      appUpgradeInProgress = true;
      appState = APP_STATE_WAIT_NOTIFICATION;
      OTA_ServerStartUpdrade(req->clientAddr, req->dataSize);

      SYS_TimerStart(&appSessionTimeoutTimer);
    }
    else
    {
      appFinishCommandProcessing(APP_UART_STATUS_UPGRADE_IN_PROGRESS);
    }
  }

  else if (APP_UART_COMMAND_BLOCK_REQUEST == command)
  {
    if (true == appUpgradeInProgress)
    {
      appState = APP_STATE_WAIT_NOTIFICATION;
      OTA_ServerSendBlock(&appUartBuffer[1]);

      SYS_TimerStop(&appSessionTimeoutTimer);
      SYS_TimerStart(&appSessionTimeoutTimer);
    }
    else
    {
      appFinishCommandProcessing(APP_UART_STATUS_NO_UPGRADE_IN_PROGRESS);
    }
  }

  else
  {
    appFinishCommandProcessing(APP_UART_STATUS_UNKNOWN_COMMAND);
  }
}

/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void)
{
  switch (appState)
  {
    case APP_STATE_INITIAL:
    {
      appInit();
    } break;

    case APP_STATE_WAIT_COMMAND:
      break;

    case APP_STATE_COMMAND_RECEIVED:
    {
      appCommandReceived();
    } break;

    case APP_STATE_WAIT_NOTIFICATION:
      break;

    default:
      break;
  }
}

/*************************************************************************//**
*****************************************************************************/
void OTA_ServerNotification(OTA_Status_t status)
{
  if (OTA_CLIENT_READY_STATUS != status)
  {
    appUpgradeInProgress = false;
    SYS_TimerStop(&appSessionTimeoutTimer);
    PHY_SetRxState(false);
  }

  appFinishCommandProcessing(status);
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
  SYS_Init();
  HAL_UartInit(38400);
  OTA_ServerInit();

  while (1)
  {
    SYS_TaskHandler();
    HAL_UartTaskHandler();
    OTA_ServerTaskHandler();
    APP_TaskHandler();
  }
}
