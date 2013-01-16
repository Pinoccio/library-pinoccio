#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "halUart.h"
#include "halSleep.h"
#include "sysTimer.h"

/*****************************************************************************
*****************************************************************************/
#ifdef NWK_ENABLE_SECURITY
  #define APP_BUFFER_SIZE     NWK_MAX_SECURED_PAYLOAD_SIZE
#else
  #define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

/*****************************************************************************
*****************************************************************************/
typedef enum AppState_t
{
  APP_STATE_INITIAL,
  APP_STATE_IDLE,
} AppState_t;

/*****************************************************************************
*****************************************************************************/
static void appSendData(void);

/*****************************************************************************
*****************************************************************************/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;

/*****************************************************************************
*****************************************************************************/
static void appDataConf(NWK_DataReq_t *req)
{
  appDataReqBusy = false;
  appSendData();
  (void)req;
}

/*****************************************************************************
*****************************************************************************/
static void appSendData(void)
{
  if (appDataReqBusy || 0 == appUartBufferPtr)
    return;

  memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);

  appDataReq.dstAddr = 1-APP_ADDR;
  appDataReq.dstEndpoint = APP_ENDPOINT;
  appDataReq.srcEndpoint = APP_ENDPOINT;
  appDataReq.options = NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = appDataReqBuffer;
  appDataReq.size = appUartBufferPtr;
  appDataReq.confirm = appDataConf;
  NWK_DataReq(&appDataReq);

  appUartBufferPtr = 0;
  appDataReqBusy = true;
}

/*****************************************************************************
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)
{
  for (uint16_t i = 0; i < bytes; i++)
  {
    uint8_t byte = HAL_UartReadByte();

    if (appUartBufferPtr == sizeof(appUartBuffer))
      appSendData();

    if (appUartBufferPtr < sizeof(appUartBuffer))
      appUartBuffer[appUartBufferPtr++] = byte;
  }
}

/*****************************************************************************
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer)
{
  appSendData();
  (void)timer;
}

/*****************************************************************************
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind)
{
  for (uint8_t i = 0; i < ind->size; i++)
    HAL_UartWriteByte(ind->data[i]);
  return true;
}

/*****************************************************************************
*****************************************************************************/
static void appInit(void)
{
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);
  NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

  // Enable RCB_BB RS232 level converter
  #if defined(PLATFORM_RCB128RFA1)
    DDRD = (1 << 4) | (1 << 6) | (1 << 7);
    PORTD = (0 << 4) | (1 << 6) | (1 << 7);
  #endif

  #if defined(PLATFORM_RCB231)
    DDRC = (1 << 4) | (1 << 6) | (1 << 7);
    PORTC = (0 << 4) | (1 << 6) | (1 << 7);
  #endif

  appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
  appTimer.mode = SYS_TIMER_PERIODIC_MODE;
  appTimer.handler = appTimerHandler;
  SYS_TimerStart(&appTimer);
}

/*****************************************************************************
*****************************************************************************/
static void APP_TaskHandler(void)
{
  switch (appState)
  {
    case APP_STATE_INITIAL:
    {
      appInit();
      appState = APP_STATE_IDLE;
    } break;

    case APP_STATE_IDLE:
      break;

    default:
      break;
  }
}

/*****************************************************************************
*****************************************************************************/
int main(void)
{
  SYS_Init();
  HAL_UartInit(38400);

  while (1)
  {
    SYS_TaskHandler();
    HAL_UartTaskHandler();
    APP_TaskHandler();
  }
}
