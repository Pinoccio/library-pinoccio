/**
 * \file otaCommon.h
 *
 * \brief OTA Server and OTA Client common interface
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
 * $Id: otaCommon.h 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

#ifndef _OTA_COMMON_H_
#define _OTA_COMMON_H_

/*- Definitions ------------------------------------------------------------*/
#define OTA_MAX_BLOCK_SIZE     90 //(NWK_MAX_PAYLOAD_SIZE - (1 + 2 + 1))
#define OTA_RESPONSE_TIMEOUT   2000
#define OTA_FRAME_SPACING      50
#define OTA_MAX_RETRIES        3

/*- Types ------------------------------------------------------------------*/
enum
{
  OTA_START_REQ_COMMAND_ID    = 0x01,
  OTA_START_RESP_COMMAND_ID   = 0x02,
  OTA_BLOCK_REQ_COMMAND_ID    = 0x03,
  OTA_BLOCK_RESP_COMMAND_ID   = 0x04,
};

typedef enum
{
  OTA_SUCCESS_STATUS           = 0x00,
  OTA_CLIENT_READY_STATUS      = 0x01,
  OTA_NETWORK_ERROR_STATUS     = 0x02,
  OTA_CRC_ERROR_STATUS         = 0x03,
  OTA_NO_RESPONSE_STATUS       = 0x04,
  OTA_SESSION_TIMEOUT_STATUS   = 0x05,
  OTA_UPGRADE_STARTED_STATUS   = 0x06,
  OTA_UPGRADE_COMPLETED_STATUS = 0x07,

  OTA_APPLICATION_STATUS_MASK  = 0x10,
  OTA_NO_SPACE_STATUS          = 0x10,
  OTA_HW_FAIL_STATUS           = 0x11,
  OTA_SW_FAIL_STATUS           = 0x12,
} OTA_Status_t;

typedef struct OtaCommandHeader_t
{
  uint8_t     commandId;
  uint16_t    sessionId;
} OtaCommandHeader_t;

typedef struct OtaStartReqCommand_t
{
  uint8_t     commandId;
  uint16_t    sessionId;
  uint32_t    size;
} OtaStartReqCommand_t;

typedef struct OtaStartRespCommand_t
{
  uint8_t     commandId;
  uint16_t    sessionId;
  uint8_t     status;
} OtaStartRespCommand_t;

typedef struct OtaBlockReqHeader_t
{
  uint8_t     commandId;
  uint16_t    sessionId;
  uint16_t    crc;
  uint8_t     size;
} OtaBlockReqHeader_t;

typedef struct OtaBlockReqCommand_t
{
  uint8_t     commandId;
  uint16_t    sessionId;
  uint16_t    crc;
  uint8_t     size;
  uint8_t     data[OTA_MAX_BLOCK_SIZE];
} OtaBlockReqCommand_t;

typedef struct OtaBlockRespCommand_t
{
  uint8_t     commandId;
  uint16_t    sessionId;
  uint8_t     status;
} OtaBlockRespCommand_t;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static inline uint16_t otaCrcUpdateCcitt(uint16_t crc, uint8_t data)
{
  data ^= crc & 0xff;
  data ^= data << 4;
  return (((uint16_t)data << 8) | ((crc >> 8) & 0xff)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3);
}

#endif // _OTA_COMMON_H_
