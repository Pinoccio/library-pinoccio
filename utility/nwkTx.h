/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_TX_H_
#define _PINOCCIO_NWK_TX_H_
#ifdef __cplusplus
extern "C"{
#endif

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "sysTypes.h"
#include "nwkRx.h"
#include "nwkFrame.h"

/*- Types ------------------------------------------------------------------*/
enum
{
  NWK_TX_CONTROL_BROADCAST_PAN_ID = 1 << 0,
  NWK_TX_CONTROL_ROUTING          = 1 << 1,
  NWK_TX_CONTROL_DIRECT_LINK      = 1 << 2,
};

/*- Prototypes -------------------------------------------------------------*/
void nwkTxInit(void);
void nwkTxFrame(NwkFrame_t *frame);
void nwkTxBroadcastFrame(NwkFrame_t *frame);
void nwkTxAckReceived(NWK_DataInd_t *ind);
void nwkTxConfirm(NwkFrame_t *frame, uint8_t status);
void nwkTxEncryptConf(NwkFrame_t *frame);
void nwkTxTaskHandler(void);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_TX_H_
