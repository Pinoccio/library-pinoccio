#ifndef _OTA_COMMON_H_
#define _OTA_COMMON_H_

/*****************************************************************************
*****************************************************************************/
#define OTA_MAX_BLOCK_SIZE     90 //(NWK_MAX_PAYLOAD_SIZE - (1 + 2 + 1))
#define OTA_RESPONSE_TIMEOUT   2000
#define OTA_FRAME_SPACING      50
#define OTA_MAX_RETRIES        3

/*****************************************************************************
*****************************************************************************/
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

/*****************************************************************************
*****************************************************************************/
static inline uint16_t otaCrcUpdateCcitt(uint16_t crc, uint8_t data)
{
  data ^= crc & 0xff;
  data ^= data << 4;
  return (((uint16_t)data << 8) | ((crc >> 8) & 0xff)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3);
}

#endif // _OTA_COMMON_H_
