#ifndef _OTA_SERVER_H_
#define _OTA_SERVER_H_

#include "nwk.h"
#include "otaCommon.h"

/*****************************************************************************
*****************************************************************************/
void OTA_ServerInit(void);
void OTA_ServerStartUpdrade(uint16_t addr, uint32_t size);
void OTA_ServerSendBlock(uint8_t *data);
void OTA_ServerNotification(OTA_Status_t status);
void OTA_ServerTaskHandler(void);

#endif // _OTA_SERVER_H_
