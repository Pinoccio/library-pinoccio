/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_SYS_CONFIG_H_
#define _PINOCCIO_SYS_CONFIG_H_
#ifdef __cplusplus
extern "C"{
#endif

//#include "config.h"

/*****************************************************************************
*****************************************************************************/
#ifndef NWK_BUFFERS_AMOUNT
#define NWK_BUFFERS_AMOUNT                       1
#endif

#ifndef NWK_MAX_ENDPOINTS_AMOUNT
#define NWK_MAX_ENDPOINTS_AMOUNT                 1
#endif

#ifndef NWK_DUPLICATE_REJECTION_TABLE_SIZE
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE       1
#endif

#ifndef NWK_DUPLICATE_REJECTION_TTL
#define NWK_DUPLICATE_REJECTION_TTL              1000 // ms
#endif

#ifndef NWK_ROUTE_TABLE_SIZE
#define NWK_ROUTE_TABLE_SIZE                     0
#endif

#ifndef NWK_ROUTE_DEFAULT_SCORE
#define NWK_ROUTE_DEFAULT_SCORE                  3
#endif

#ifndef NWK_ACK_WAIT_TIME
#define NWK_ACK_WAIT_TIME                        1000 // ms
#endif

//#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY

#ifndef SYS_SECURITY_MODE
#define SYS_SECURITY_MODE                        0
#endif

#ifdef HAL_ENABLE_UART
#define HAL_UART_CHANNEL                    0
#define HAL_UART_RX_FIFO_SIZE               200
#define HAL_UART_TX_FIFO_SIZE               200
#endif

/*****************************************************************************
*****************************************************************************/
#if defined(NWK_ENABLE_SECURITY) && (SYS_SECURITY_MODE == 0)
  #define PHY_ENABLE_AES_MODULE
#endif

#ifdef NWK_ENABLE_SECURITY
  #define APP_BUFFER_SIZE     NWK_MAX_SECURED_PAYLOAD_SIZE
#else
  #define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_SYS_CONFIG_H_