#ifndef _OTA_CLIENT_H_
#define _OTA_CLIENT_H_

#include "nwk.h"
#include "otaCommon.h"

/*****************************************************************************
*****************************************************************************/
void OTA_ClientInit(void);
void OTA_ClientNotification(OTA_Status_t status);
void OTA_ClientBlockIndication(uint8_t size, uint8_t *data);
void OTA_ClientBlockConfirm(OTA_Status_t status);
void OTA_ClientTaskHandler(void);

#endif // _OTA_CLIENT_H_
