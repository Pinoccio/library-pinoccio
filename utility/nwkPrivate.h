/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_NWK_PRIVATE_H_
#define _PINOCCIO_NWK_PRIVATE_H_

#include <stdint.h>
#include "nwk.h"
#include "sysTypes.h"

/*****************************************************************************
*****************************************************************************/
#define NWK_SECURITY_MIC_SIZE    4
#define NWK_SECURITY_KEY_SIZE    16
#define NWK_SECURITY_BLOCK_SIZE  16

#ifdef __cplusplus
extern "C"{
#endif

/*****************************************************************************
*****************************************************************************/
enum
{
  NWK_COMMAND_ACK              = 0x00,
  NWK_COMMAND_ROUTE_ERROR      = 0x01,
};

enum
{
  NWK_TX_CONTROL_BROADCAST_PAN_ID = 1 << 0,
  NWK_TX_CONTROL_ROUTING          = 1 << 1,
};

/*****************************************************************************
*****************************************************************************/
typedef struct PACK NwkFrameHeader_t
{
  uint16_t    macFcf;
  uint8_t     macSeq;
  uint16_t    macDstPanId;
  uint16_t    macDstAddr;
  uint16_t    macSrcAddr;

  struct PACK
  {
    uint8_t   ackRequest       : 1;
    uint8_t   securityEnabled  : 1;
    uint8_t   linkLocal        : 1;
    uint8_t   reserved         : 5;
  }           nwkFcf;
  uint8_t     nwkSeq;
  uint16_t    nwkSrcAddr;
  uint16_t    nwkDstAddr;
  struct PACK
  {
    uint8_t   nwkSrcEndpoint   : 4;
    uint8_t   nwkDstEndpoint   : 4;
  };
} NwkFrameHeader_t;

typedef struct NwkFrame_t
{
  uint8_t            state;
  uint8_t            size;
  struct PACK
  {
    NwkFrameHeader_t header;
    uint8_t          payload[NWK_MAX_PAYLOAD_SIZE];
  } data;

  union
  {
    struct
    {
      uint8_t        lqi;
      int8_t         rssi;
    } rx;

    struct
    {
      uint8_t        status;
      uint16_t       timeout;
      uint8_t        control;
      void           (*confirm)(struct NwkFrame_t *frame);
    } tx;
  };
} NwkFrame_t;

typedef struct PACK NwkAckCommand_t
{
  uint8_t    id;
  uint8_t    seq;
  uint8_t    control;
} NwkAckCommand_t;

typedef struct PACK NwkRouteErrorCommand_t
{
  uint8_t    id;
  uint16_t   srcAddr;
  uint16_t   dstAddr;
} NwkRouteErrorCommand_t;

typedef struct NwkIb_t
{
  uint16_t     addr;
  uint16_t     panId;
  uint8_t      nwkSeqNum;
  uint8_t      macSeqNum;
  bool         (*endpoint[NWK_MAX_ENDPOINTS_AMOUNT])(NWK_DataInd_t *ind);
#ifdef NWK_ENABLE_SECURITY
  uint32_t     key[4];
#endif
} NwkIb_t;

/*****************************************************************************
*****************************************************************************/
extern NwkIb_t nwkIb;

/*****************************************************************************
*****************************************************************************/
void nwkFrameInit(void);
NwkFrame_t *nwkFrameAlloc(uint8_t size);
void nwkFrameFree(NwkFrame_t *frame);
NwkFrame_t *nwkFrameByIndex(uint8_t i);
void nwkFrameCommandInit(NwkFrame_t *frame);

void nwkRxInit(void);
bool nwkRxBusy(void);
void nwkRxDecryptConf(NwkFrame_t *frame, bool status);
void nwkRxTaskHandler(void);

void nwkTxInit(void);
void nwkTxFrame(NwkFrame_t *frame);
void nwkTxBroadcastFrame(NwkFrame_t *frame);
void nwkTxAckReceived(NWK_DataInd_t *ind);
bool nwkTxBusy(void);
void nwkTxEncryptConf(NwkFrame_t *frame);
void nwkTxTaskHandler(void);

void nwkDataReqInit(void);
bool nwkDataReqBusy(void);
void nwkDataReqTaskHandler(void);

#ifdef NWK_ENABLE_ROUTING
void nwkRouteInit(void);
void nwkRouteRemove(uint16_t dst);
void nwkRouteFrameReceived(NwkFrame_t *frame);
void nwkRouteFrameSent(NwkFrame_t *frame);
uint16_t nwkRouteNextHop(uint16_t dst);
void nwkRouteFrame(NwkFrame_t *frame);
void nwkRouteErrorReceived(NWK_DataInd_t *ind);
#endif

#ifdef NWK_ENABLE_SECURITY
void nwkSecurityInit(void);
void nwkSecurityProcess(NwkFrame_t *frame, bool encrypt);
void nwkSecurityTaskHandler(void);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_PRIVATE_H_