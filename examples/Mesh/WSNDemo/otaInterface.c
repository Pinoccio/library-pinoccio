#include "sysTypes.h"
#include "otaClient.h"

#ifdef APP_ENABLE_OTA

/*****************************************************************************
*****************************************************************************/
#define STARTING_ADDRESS        (SYS_DEVICE_SIZE / 2)
#define IAP_SWITCH_HANDLER      (SYS_DEVICE_SIZE - 4)
#define IAP_WRITE_PAGE_HANDLER  (SYS_DEVICE_SIZE - 2)

/*****************************************************************************
*****************************************************************************/
typedef void (*IapWritePage_t)(uint32_t addr, uint16_t *buf);
typedef void (*IapSwitch_t)(void);

/*****************************************************************************
*****************************************************************************/
static bool appUpgradeInProgress = false;
static uint32_t appOtaAddr;
static uint8_t appOtaPage[SYS_PAGE_SIZE];
static uint16_t appOtaSize;

/*****************************************************************************
*****************************************************************************/
static void *appGetIapHandler(uint32_t addr)
{
  void *handler;

#if defined(__ICCAVR__)
  memcpy_P((void *)&handler, (void const __farflash *)addr, sizeof(void *));
#else
  handler = (void *)pgm_read_word_far(addr);
#endif

  return handler;
}

/*****************************************************************************
*****************************************************************************/
static void appWritePage(void)
{
  IapWritePage_t iapWritePage = (IapWritePage_t)appGetIapHandler(IAP_WRITE_PAGE_HANDLER);

  if (0 == appOtaSize)
    return;

  for (uint16_t i = appOtaSize; i < sizeof(appOtaPage); i++)
    appOtaPage[i] = 0xff;

  ATOMIC_SECTION_ENTER
    iapWritePage(appOtaAddr, (uint16_t *)appOtaPage);
  ATOMIC_SECTION_LEAVE
}

/*****************************************************************************
*****************************************************************************/
static void appSwitch(void)
{
  IapSwitch_t iapSwitch = (IapSwitch_t)appGetIapHandler(IAP_SWITCH_HANDLER);

  ATOMIC_SECTION_ENTER
    iapSwitch();
  ATOMIC_SECTION_LEAVE
}

/*****************************************************************************
*****************************************************************************/
void OTA_ClientNotification(OTA_Status_t status)
{
  if (OTA_UPGRADE_STARTED_STATUS == status)
  {
    if (appUpgradeInProgress)
      return; // Should not happen at all

    appUpgradeInProgress = true;
    appOtaAddr = STARTING_ADDRESS;
    appOtaSize = 0;
  }
  else if (OTA_UPGRADE_COMPLETED_STATUS == status)
  {
    appWritePage();
    appSwitch();
    appUpgradeInProgress = false;
  }
  else
  {
    // Some error happened, upgrade will be stopped.
    appUpgradeInProgress = false;
  }
}

/*****************************************************************************
*****************************************************************************/
void OTA_ClientBlockIndication(uint8_t size, uint8_t *data)
{
  if (appUpgradeInProgress)
  {
    while (size > 0)
    {
      for (; (size > 0) && (appOtaSize < sizeof(appOtaPage)); appOtaSize++, size--)
        appOtaPage[appOtaSize] = *data++;

      if (sizeof(appOtaPage) == appOtaSize)
      {
        appWritePage();
        appOtaAddr += sizeof(appOtaPage);
        appOtaSize = 0;
      }
    }
    OTA_ClientBlockConfirm(OTA_SUCCESS_STATUS);
  }
  else
  {
    OTA_ClientBlockConfirm(OTA_SW_FAIL_STATUS);
  }
}

#endif // APP_ENABLE_OTA
