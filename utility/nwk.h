/**
 * \file nwk.h
 *
 * \brief Network layer public interface
 *
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
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
 * $Id: nwk.h 5223 2012-09-10 16:47:17Z ataradov $
 *
 */

#ifndef _PINOCCIO_NWK_H_
#define _PINOCCIO_NWK_H_
#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "sysConfig.h"

/*****************************************************************************
*****************************************************************************/
#define NWK_MAX_PAYLOAD_SIZE            (127 - 16/*NwkFrameHeader_t*/ - 2/*crc*/)
#define NWK_MAX_SECURED_PAYLOAD_SIZE    (127 - 16/*NwkFrameHeader_t*/ - 2/*crc*/ - 4/*mic*/)

/*****************************************************************************
*****************************************************************************/
typedef enum
{
  NWK_SUCCESS_STATUS                      = 0x00,
  NWK_ERROR_STATUS                        = 0x01,
  NWK_OUT_OF_MEMORY_STATUS                = 0x02,

  NWK_NO_ACK_STATUS                       = 0x10,

  NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS   = 0x20,
  NWK_PHY_NO_ACK_STATUS                   = 0x21,
} NWK_Status_t;

enum
{
  NWK_OPT_ACK_REQUEST          = 1 << 0,
  NWK_OPT_ENABLE_SECURITY      = 1 << 1,
  NWK_OPT_BROADCAST_PAN_ID     = 1 << 2,
  NWK_OPT_LINK_LOCAL           = 1 << 3,
};

enum
{
  NWK_IND_OPT_ACK_REQUESTED     = 1 << 0,
  NWK_IND_OPT_SECURED           = 1 << 1,
  NWK_IND_OPT_BROADCAST         = 1 << 2,
  NWK_IND_OPT_LOCAL             = 1 << 3,
  NWK_IND_OPT_BROADCAST_PAN_ID  = 1 << 4,
  NWK_IND_OPT_LINK_LOCAL        = 1 << 5,
};

enum
{
  NWK_ACK_CONTROL_NONE         = 0,
  NWK_ACK_CONTROL_PENDING      = 1 << 0,
};

typedef struct NWK_DataReq_t
{
  // service fields
  void         *next;
  void         *frame;
  uint8_t      state;

  // request parameters
  uint16_t     dstAddr;
  uint8_t      dstEndpoint;
  uint8_t      srcEndpoint;
  uint8_t      options;

  uint8_t      *data;
  uint8_t      size;

  void         (*confirm)(struct NWK_DataReq_t *req);

  // confirmation parameters
  uint8_t      status;
  uint8_t      control;
} NWK_DataReq_t;

typedef struct NWK_DataInd_t
{
  uint16_t     srcAddr;
  uint8_t      srcEndpoint;
  uint8_t      dstEndpoint;
  uint8_t      options;
  uint8_t      *data;
  uint8_t      size;
  uint8_t      lqi;
  int8_t       rssi;
} NWK_DataInd_t;

/*****************************************************************************
*****************************************************************************/
void NWK_Init(void);
void NWK_SetAddr(uint16_t addr);
void NWK_SetPanId(uint16_t panId);
void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind));
void NWK_OpenEndpoint(uint8_t id, FuncDelegate1 delegate);
#ifdef NWK_ENABLE_SECURITY
void NWK_SetSecurityKey(uint8_t *key);
#endif
bool NWK_Busy(void);
void NWK_SleepReq(void);
void NWK_WakeupReq(void);
void NWK_DataReq(NWK_DataReq_t *req);
void NWK_SetAckControl(uint8_t control);
void NWK_TaskHandler(void);

#ifdef NWK_ENABLE_ROUTING
uint16_t NWK_RouteNextHop(uint16_t dst);
#endif

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _PINOCCIO_NWK_H_

