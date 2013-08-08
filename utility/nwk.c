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
#include "phy.h"
#include "sysConfig.h"
#include "nwkRx.h"
#include "nwkTx.h"
#include "nwkGroup.h"
#include "nwkFrame.h"
#include "nwkRoute.h"
#include "nwkSecurity.h"
#include "nwkRouteDiscovery.h"

/*- Variables --------------------------------------------------------------*/
NwkIb_t nwkIb;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
  @brief Initializes all network layer modules
*****************************************************************************/
void NWK_Init(void)
{
  nwkIb.nwkSeqNum = 0;
  nwkIb.macSeqNum = 0;
  nwkIb.addr = 0;

  uint8_t i = 0;
  
  for (i=0; i < NWK_ENDPOINTS_AMOUNT; i++)
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

#ifdef NWK_ENABLE_MULTICAST
  nwkGroupInit();
#endif

#ifdef NWK_ENABLE_ROUTE_DISCOVERY
  nwkRouteDiscoveryInit();
#endif
}

/*************************************************************************//**
  @brief Sets network address of the node
  @param[in] addr Adddress to set
*****************************************************************************/
void NWK_SetAddr(uint16_t addr)
{
  nwkIb.addr = addr;
  PHY_SetShortAddr(addr);
}

/*************************************************************************//**
  @brief Sets network identifier (PAN) of the node
  @param[in] panId PAN ID to set
*****************************************************************************/
void NWK_SetPanId(uint16_t panId)
{
  nwkIb.panId = panId;
  PHY_SetPanId(panId);
}

/*************************************************************************//**
  @brief Registers callback @a ind for the endpoint @a endpoint
  @param[in] id Endpoint index (1-15)
  @param[in] handler Pointer to the callback function
*****************************************************************************/
void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind))
{
  nwkIb.endpoint[id] = handler;
}

/*************************************************************************//**
  @brief Checks if network layer is ready for sleep
  @return @c true if network layer is ready for sleep or @c false otherwise
*****************************************************************************/
bool NWK_Busy(void)
{
  return PHY_Busy() || (NULL != nwkFrameNext(NULL));
}

/*************************************************************************//**
  @brief Puts network layer to a sleeping state
*****************************************************************************/
void NWK_SleepReq(void)
{
  PHY_Sleep();
}

/*************************************************************************//**
  @brief Puts network layer to an active state
*****************************************************************************/
void NWK_WakeupReq(void)
{
  PHY_Wakeup();
}

/*************************************************************************//**
  @brief Calculates linearized value for the given value of the LQI
  @param[in] lqi LQI value as provided by the transceiver
  @return linearized value directly proportional to the probability of delivery
*****************************************************************************/
uint8_t NWK_LinearizeLqi(uint8_t lqi)
{
  const uint8_t val[] = { 3, 8, 26, 64, 128, 190, 230, 247, 252 };
  uint8_t cl = 25;
  uint8_t i = 0;
  
  for (i=0; i < sizeof(val); i++)
  {
    if (lqi < cl)
      return val[i];
    cl += 25;
  }

  return 255;
}

/*************************************************************************//**
  @brief Network layer task handler
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
