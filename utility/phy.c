/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

/*- Includes ---------------------------------------------------------------*/
#include "sysTypes.h"
#if  defined(__AVR_ATmega128RFA1__)
#include "atmega128rfa1.h"
#elif defined(__AVR_ATmega256RFR2__)
#include "atmega256rfr2.h"
#endif
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
  uint8_t     band;
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

#ifdef _ATMEGA256RFR2_H_
  TRX_RPC_REG = 0xff;
#endif

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
  phyIb.band = 0;
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
void PHY_SetBand(uint8_t band)
{
  phyIb.request |= PHY_REQ_CHANNEL;
  phyIb.band = band;
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
  uint8_t i = 0;
  
  for (i = 0; i < size; i++)
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
  uint8_t i;
  
  for (i = 0; i < 16; i += 2)
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

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
/*************************************************************************//**
*****************************************************************************/
void PHY_RandomConf(uint16_t rnd)
{
 srand(rnd);
}
#endif

#ifdef PHY_ENABLE_AES_MODULE
/*************************************************************************//**
*****************************************************************************/
static void phyEncryptBlock(void)
{
  uint8_t i = 0;
  for (i = 0; i < AES_BLOCK_SIZE; i++)
    AES_KEY = phyIb.key[i];

  AES_CTRL = (0 << AES_CTRL_DIR) | (0 << AES_CTRL_MODE);

  for (i = 0; i < AES_BLOCK_SIZE; i++)
    AES_STATE = phyIb.text[i];

  AES_CTRL |= (1 << AES_CTRL_REQUEST);

  while (0 == (AES_STATUS & (1 << AES_STATUS_RY)));

  for (i = 0; i < AES_BLOCK_SIZE; i++)
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
    #if defined(__AVR_ATmega128RFA1__)
    PHY_CC_CCA_REG_s.channel = phyIb.channel;
    
    #elif defined(__AVR_ATmega256RFR2__)
    CC_CTRL_1_REG_s.ccBand = phyIb.band;

    if (0 == phyIb.band)
      PHY_CC_CCA_REG_s.channel = phyIb.channel;
    else
      CC_CTRL_0_REG = phyIb.channel;
    #endif
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
      uint8_t i = 0;
      
      for (i = 0; i < phyRxSize + 1/*lqi*/; i++)
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
