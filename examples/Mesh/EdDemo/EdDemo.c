#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "halUart.h"
#include "sysTimer.h"

/*****************************************************************************
*****************************************************************************/
typedef enum AppState_t
{
  APP_STATE_INITIAL,
  APP_STATE_START_ED,
  APP_STATE_WAIT_ED_CONF,
  APP_STATE_NEXT_CHANNEL,
  APP_STATE_WAIT_SCAN_TIMER,
} AppState_t;

/*****************************************************************************
*****************************************************************************/
static AppState_t appState = APP_STATE_INITIAL;
static uint8_t channel;
static uint8_t edValue[APP_LAST_CHANNEL - APP_FIRST_CHANNEL + 1];
static SYS_Timer_t appScanTimer;

/*****************************************************************************
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)
{
  for (uint16_t i = 0; i < bytes; i++)
    HAL_UartReadByte();
}

/*****************************************************************************
*****************************************************************************/
static void appPrintEdValues(void)
{
  char hex[] = "0123456789abcdef";

  for (uint8_t i = 0; i < sizeof(edValue); i++)
  {
    uint8_t v = edValue[i] - PHY_RSSI_BASE_VAL;
    HAL_UartWriteByte(hex[(v >> 4) & 0x0f]);
    HAL_UartWriteByte(hex[v & 0x0f]);
    HAL_UartWriteByte(' ');
  }

  HAL_UartWriteByte('\r');
  HAL_UartWriteByte('\n');
}

/*****************************************************************************
*****************************************************************************/
void PHY_EdConf(int8_t ed)
{
  edValue[channel - APP_FIRST_CHANNEL] = ed;
  appState = APP_STATE_NEXT_CHANNEL;
}

/*****************************************************************************
*****************************************************************************/
static void appScanTimerHandler(SYS_Timer_t *timer)
{
  appState = APP_STATE_START_ED;
  (void)timer;
}

/*****************************************************************************
*****************************************************************************/
static void appInit(void)
{
  // Enable RCB_BB RS232 level converter
  #ifdef PLATFORM_RCB128RFA1
    DDRD = (1 << 4) | (1 << 6) | (1 << 7);
    PORTD = (0 << 4) | (1 << 6) | (1 << 7);
  #endif

  #ifdef PLATFORM_RCB231
    DDRC = (1 << 4) | (1 << 6) | (1 << 7);
    PORTC = (0 << 4) | (1 << 6) | (1 << 7);
  #endif

  appScanTimer.interval = APP_SCAN_INTERVAL;
  appScanTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appScanTimer.handler = appScanTimerHandler;

  appState = APP_STATE_START_ED;
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
    } break;

    case APP_STATE_START_ED:
    {
      channel = APP_FIRST_CHANNEL;
      PHY_SetChannel(channel);
      PHY_EdReq();
      appState = APP_STATE_WAIT_ED_CONF;
    } break;

    case APP_STATE_NEXT_CHANNEL:
    {
      if (APP_LAST_CHANNEL == channel)
      {
        appPrintEdValues();
        SYS_TimerStart(&appScanTimer);
        appState = APP_STATE_WAIT_SCAN_TIMER;
      }
      else
      {
        channel++;
        PHY_SetChannel(channel);
        PHY_EdReq();
        appState = APP_STATE_WAIT_ED_CONF;
      }
    } break;

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
