/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_ROUTE_DISCOVERY_H_
#define _PINOCCIO_NWK_ROUTE_DISCOVERY_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "nwk.h"
#include "sysTypes.h"
#include "nwkFrame.h"

#ifdef NWK_ENABLE_ROUTE_DISCOVERY

/*- Prototypes -------------------------------------------------------------*/
void nwkRouteDiscoveryInit(void);
void nwkRouteDiscoveryRequest(NwkFrame_t *frame);
bool nwkRouteDiscoveryReplyReceived(NWK_DataInd_t *ind);
bool nwkRouteDiscoveryRequestReceived(NWK_DataInd_t *ind);

#endif // NWK_ENABLE_ROUTE_DISCOVERY

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_ROUTE_DISCOVERY_H_
