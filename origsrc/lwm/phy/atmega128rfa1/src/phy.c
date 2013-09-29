/**
 * \file phy.c
 *
 * \brief ATMEGA128RFA1 PHY implementation
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
 * $Id: phy.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

#ifdef PHY_ATMEGA128RFA1

/*- Includes ---------------------------------------------------------------*/
#include "sysTypes.h"
#include "atmega128rfa1.h"
#include "phy.h"
#include "hal.h"

/*- Definitions ------------------------------------------------------------*/
#define IRQ_STATUS_CLEAR_VALUE         0xff
#define RANDOM_NUMBER_UPDATE_INTERVAL  1 // us

/*- Types ------------------------------------------------------------------*/
typedef enum PhyState_t
{
  PHY_STATE_INITIAL,
  PHY_STATE_IDLE,
  PHY_STATE_SLEEP,
  PHY_STATE_TX_WAIT_END,
  PHY_STATE_TX_CONFIRM,
  PHY_STATE_RX_IND,
  PHY_STATE_ED_WAIT,
  PHY_STATE_ED_DONE,
} PhyState_t;

enum
{
  PHY_REQ_NONE     = 0,
  PHY_REQ_CHANNEL  = (1 << 0),
  PHY_REQ_PANID    = (1 << 1),
  PHY_REQ_ADDR     = (1 << 2),
  PHY_REQ_TX_POWER = (1 << 3),
  PHY_REQ_RX       = (1 << 4),
  PHY_REQ_RANDOM   = (1 << 5),
  PHY_REQ_ENCRYPT  = (1 << 6),
  PHY_REQ_ED       = (1 << 7),
};

typedef struct PhyIb_t
{
  uint8_t     request;

  uint8_t     channel;
  uint16_t    panId;
  uint16_t    addr;
  uint8_t     txPower;
  bool        rx;
#ifdef PHY_ENABLE_AES_MODULE
  uint8_t     *text;
  uint8_t     *key;
#endif
} PhyIb_t;

/*- Prototypes -------------------------------------------------------------*/
static inline void phyTrxSetState(uint8_t state);
static void phySetRxState(void);
#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
static uint16_t phyGetRandomNumber(void);
#endif

/*- Variables --------------------------------------------------------------*/
static PhyIb_t              phyIb;
static volatile PhyState_t  phyState = PHY_STATE_INITIAL;
static volatile uint8_t     phyTxStatus;
static volatile int8_t      phyRxRssi;
static volatile uint8_t     phyRxSize;
static uint8_t              phyRxBuffer[128];

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void PHY_Init(void)
{
  TRXPR_REG_s.trxrst = 1;

  phyTrxSetState(TRX_CMD_TRX_OFF);

  CSMA_SEED_1_REG_s.aackSetPd = 1;
  CSMA_SEED_1_REG_s.aackDisAck = 0;

  IRQ_STATUS_REG = IRQ_STATUS_CLEAR_VALUE;
  IRQ_MASK_REG_s.rxEndEn = 1;
  IRQ_MASK_REG_s.txEndEn = 1;

  TRX_CTRL_2_REG_s.rxSafeMode = 1;

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
  CSMA_SEED_0_REG = (uint8_t)phyGetRandomNumber();
#else
  CSMA_SEED_0_REG = 0x11;
#endif

  phyIb.request = PHY_REQ_NONE;
  phyIb.rx = false;
  phyState = PHY_STATE_IDLE;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_SetRxState(bool rx)
{
  phyIb.request |= PHY_REQ_RX;
  phyIb.rx = rx;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_SetChannel(uint8_t channel)
{
  phyIb.request |= PHY_REQ_CHANNEL;
  phyIb.channel = channel;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_SetPanId(uint16_t panId)
{
  phyIb.request |= PHY_REQ_PANID;
  phyIb.panId = panId;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_SetShortAddr(uint16_t addr)
{
  phyIb.request |= PHY_REQ_ADDR;
  phyIb.addr = addr;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_SetTxPower(uint8_t txPower)
{
  phyIb.request |= PHY_REQ_TX_POWER;
  phyIb.txPower = txPower;
}

/*************************************************************************//**
*****************************************************************************/
bool PHY_Busy(void)
{
  return PHY_STATE_IDLE != phyState || PHY_REQ_NONE != phyIb.request;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_Sleep(void)
{
  phyTrxSetState(TRX_CMD_TRX_OFF);
  TRXPR_REG_s.slptr = 1;
  phyState = PHY_STATE_SLEEP;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_Wakeup(void)
{
  TRXPR_REG_s.slptr = 0;
  phySetRxState();
  phyState = PHY_STATE_IDLE;
}

/*************************************************************************//**
*****************************************************************************/
void PHY_DataReq(uint8_t *data, uint8_t size)
{
  phyTrxSetState(TRX_CMD_TX_ARET_ON);

  TRX_FRAME_BUFFER(0) = size + 2/*crc*/;
  for (uint8_t i = 0; i < size; i++)
    TRX_FRAME_BUFFER(i+1) = data[i];

  TRX_STATE_REG = TRX_CMD_TX_START;

  phyState = PHY_STATE_TX_WAIT_END;
}

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
/*************************************************************************//**
*****************************************************************************/
void PHY_RandomReq(void)
{
  phyIb.request |= PHY_REQ_RANDOM;
}
#endif

#ifdef PHY_ENABLE_AES_MODULE
/*************************************************************************//**
*****************************************************************************/
void PHY_EncryptReq(uint8_t *text, uint8_t *key)
{
  phyIb.request |= PHY_REQ_ENCRYPT;
  phyIb.text = text;
  phyIb.key = key;
}
#endif

#ifdef PHY_ENABLE_ENERGY_DETECTION
/*************************************************************************//**
*****************************************************************************/
void PHY_EdReq(void)
{
  phyIb.request |= PHY_REQ_ED;
}
#endif

/*************************************************************************//**
*****************************************************************************/
ISR(TRX24_TX_END_vect)
{
  if (TRX_STATUS_TX_ARET_ON == TRX_STATUS_REG_s.trxStatus)
  {
    phyTxStatus = TRX_STATE_REG_s.tracStatus;
    TRX_STATE_REG = TRX_CMD_PLL_ON; // Don't wait for this to complete
    phyState = PHY_STATE_TX_CONFIRM;
  }
  else
  {
    // Auto ACK transmission completed
  }
}

/*************************************************************************//**
*****************************************************************************/
ISR(TRX24_RX_END_vect)
{
  TRX_STATE_REG = TRX_CMD_PLL_ON; // Don't wait for this to complete
  phyRxRssi = (int8_t)PHY_ED_LEVEL_REG;
  phyRxSize = TST_RX_LENGTH_REG;
  phyState = PHY_STATE_RX_IND;
}

#ifdef PHY_ENABLE_ENERGY_DETECTION
/*************************************************************************//**
*****************************************************************************/
ISR(TRX24_CCA_ED_DONE_vect)
{
  phyRxRssi = (int8_t)PHY_ED_LEVEL_REG;
  phyState = PHY_STATE_ED_DONE;
}
#endif

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
/*************************************************************************//**
*****************************************************************************/
static uint16_t phyGetRandomNumber(void)
{
  uint16_t rnd = 0;

  IRQ_MASK_REG = 0x00;
  phyTrxSetState(TRX_CMD_RX_ON);

  for (uint8_t i = 0; i < 16; i += 2)
  {
    HAL_Delay(RANDOM_NUMBER_UPDATE_INTERVAL);
    rnd |= PHY_RSSI_REG_s.rndValue << i;
  }

  phyTrxSetState(TRX_CMD_TRX_OFF);

  IRQ_STATUS_REG = IRQ_STATUS_CLEAR_VALUE;
  IRQ_MASK_REG_s.rxEndEn = 1;
  IRQ_MASK_REG_s.txEndEn = 1;

  return rnd;
}
#endif

#ifdef PHY_ENABLE_AES_MODULE
/*************************************************************************//**
*****************************************************************************/
static void phyEncryptBlock(void)
{
  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++)
    AES_KEY = phyIb.key[i];

  AES_CTRL = (0 << AES_CTRL_DIR) | (0 << AES_CTRL_MODE);

  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++)
    AES_STATE = phyIb.text[i];

  AES_CTRL |= (1 << AES_CTRL_REQUEST);

  while (0 == (AES_STATUS & (1 << AES_STATUS_RY)));

  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++)
    phyIb.text[i] = AES_STATE;
}
#endif

/*************************************************************************//**
*****************************************************************************/
static void phySetRxState(void)
{
  if (phyIb.rx)
    phyTrxSetState(TRX_CMD_RX_AACK_ON);
  else
    phyTrxSetState(TRX_CMD_TRX_OFF);
}

/*************************************************************************//**
*****************************************************************************/
static void phyHandleSetRequests(void)
{
  phyTrxSetState(TRX_CMD_TRX_OFF);

  if (phyIb.request & PHY_REQ_CHANNEL)
  {
    PHY_CC_CCA_REG_s.channel = phyIb.channel;
  }

  if (phyIb.request & PHY_REQ_PANID)
  {
    uint8_t *d = (uint8_t *)&phyIb.panId;
    PAN_ID_0_REG = d[0];
    PAN_ID_1_REG = d[1];
  }

  if (phyIb.request & PHY_REQ_ADDR)
  {
    uint8_t *d = (uint8_t *)&phyIb.addr;
    SHORT_ADDR_0_REG = d[0];
    SHORT_ADDR_1_REG = d[1];
  }

  if (phyIb.request & PHY_REQ_TX_POWER)
  {
    PHY_TX_PWR_REG_s.txPwr = phyIb.txPower;
  }

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
  if (phyIb.request & PHY_REQ_RANDOM)
  {
    uint16_t rnd = phyGetRandomNumber();
    PHY_RandomConf(rnd);
  }
#endif

#ifdef PHY_ENABLE_AES_MODULE
  if (phyIb.request & PHY_REQ_ENCRYPT)
  {
    phyEncryptBlock();
    PHY_EncryptConf();
  }
#endif

#ifdef PHY_ENABLE_ENERGY_DETECTION
  if (phyIb.request & PHY_REQ_ED)
  {
    IRQ_MASK_REG_s.rxEndEn = 0;
    IRQ_MASK_REG_s.txEndEn = 0;
    IRQ_MASK_REG_s.ccaEdReadyEn = 1;

    phyTrxSetState(TRX_CMD_RX_ON);
    PHY_ED_LEVEL_REG = 0;
    phyState = PHY_STATE_ED_WAIT;
  }
#endif

#ifdef PHY_ENABLE_ENERGY_DETECTION
  if (!(phyIb.request & PHY_REQ_ED))
    phySetRxState();
#else
  phySetRxState();
#endif

  phyIb.request = PHY_REQ_NONE;
}

/*************************************************************************//**
*****************************************************************************/
static inline void phyTrxSetState(uint8_t state)
{
  TRX_STATE_REG = TRX_CMD_FORCE_TRX_OFF;
  while (TRX_STATUS_TRX_OFF != TRX_STATUS_REG_s.trxStatus);

  TRX_STATE_REG = state;
  while (state != TRX_STATUS_REG_s.trxStatus);
}

/*************************************************************************//**
*****************************************************************************/
void PHY_TaskHandler(void)
{
  switch (phyState)
  {
    case PHY_STATE_IDLE:
    {
      if (phyIb.request)
        phyHandleSetRequests();
    } break;

    case PHY_STATE_TX_CONFIRM:
    {
      PHY_DataConf(phyTxStatus);

      while (TRX_CMD_PLL_ON != TRX_STATUS_REG_s.trxStatus);
      phyState = PHY_STATE_IDLE;
      phySetRxState();
    } break;

    case PHY_STATE_RX_IND:
    {
      PHY_DataInd_t ind;

      for (uint8_t i = 0; i < phyRxSize + 1/*lqi*/; i++)
        phyRxBuffer[i] = TRX_FRAME_BUFFER(i);

      ind.data = phyRxBuffer;
      ind.size = phyRxSize - 2/*crc*/;
      ind.lqi  = phyRxBuffer[phyRxSize];
      ind.rssi = phyRxRssi + PHY_RSSI_BASE_VAL;
      PHY_DataInd(&ind);

      while (TRX_CMD_PLL_ON != TRX_STATUS_REG_s.trxStatus);
      phyState = PHY_STATE_IDLE;
      phySetRxState();
    } break;

#ifdef PHY_ENABLE_ENERGY_DETECTION
    case PHY_STATE_ED_DONE:
    {
      PHY_EdConf(phyRxRssi + PHY_RSSI_BASE_VAL);

      IRQ_STATUS_REG = IRQ_STATUS_CLEAR_VALUE;
      IRQ_MASK_REG_s.rxEndEn = 1;
      IRQ_MASK_REG_s.txEndEn = 1;
      IRQ_MASK_REG_s.ccaEdReadyEn = 0;

      phyState = PHY_STATE_IDLE;
      phySetRxState();
    } break;
#endif

    default:
      break;
  }
}

#endif // PHY_ATMEGA128RFA1
