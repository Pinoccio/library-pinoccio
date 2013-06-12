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
#include "phy.h"

/*****************************************************************************
*****************************************************************************/
NwkIb_t nwkIb;

/*****************************************************************************
*****************************************************************************/
void NWK_Init(void)
{
  nwkIb.nwkSeqNum = 0;
  nwkIb.macSeqNum = 0;
  nwkIb.addr = 0;
  uint8_t i = 0;

  for (i; i < NWK_MAX_ENDPOINTS_AMOUNT; i++)
    nwkIb.endpoint[i] = NULL;

  nwkTxInit();
  nwkRxInit();
  nwkFrameInit();
  nwkDataReqInit();

#ifdef NWK_ENABLE_ROUTING
  nwkRouteInit();
#endif

#ifdef NWK_ENABLE_SECURITY
  nwkSecurityInit();
#endif
}

/*****************************************************************************
*****************************************************************************/
void NWK_SetAddr(uint16_t addr)
{
  nwkIb.addr = addr;
  PHY_SetShortAddr(addr);
}

/*****************************************************************************
*****************************************************************************/
void NWK_SetPanId(uint16_t panId)
{
  nwkIb.panId = panId;
  PHY_SetPanId(panId);
}

/*****************************************************************************
*****************************************************************************/
void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind))
{
  nwkIb.endpoint[id] = handler;
}

/*****************************************************************************
*****************************************************************************/
/*
void NWK_OpenEndpoint(uint8_t id, FuncDelegate1 delegate)
{
  nwkIb.endpoint[id] = delegate;
}*/


#ifdef NWK_ENABLE_SECURITY
/*****************************************************************************
*****************************************************************************/
void NWK_SetSecurityKey(uint8_t *key)
{
  memcpy((uint8_t *)nwkIb.key, key, NWK_SECURITY_KEY_SIZE);
}
#endif

/*****************************************************************************
*****************************************************************************/
bool NWK_Busy(void)
{
  return nwkRxBusy() || nwkTxBusy() || nwkDataReqBusy() || PHY_Busy();
}

/*****************************************************************************
*****************************************************************************/
void NWK_SleepReq(void)
{
  PHY_Sleep();
}

/*****************************************************************************
*****************************************************************************/
void NWK_WakeupReq(void)
{
  PHY_Wakeup();
}

/*****************************************************************************
*****************************************************************************/
void NWK_TaskHandler(void)
{
  nwkRxTaskHandler();
  nwkTxTaskHandler();
  nwkDataReqTaskHandler();
#ifdef NWK_ENABLE_SECURITY
  nwkSecurityTaskHandler();
#endif
}