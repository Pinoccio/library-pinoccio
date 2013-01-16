/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012, Atmel Corporation. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the ASF license as described in license.txt.         *
\**************************************************************************/

#include <stdint.h>
#include <string.h>
#include "sysEncrypt.h"
#include "sysConfig.h"
#include "phy.h"

#ifdef NWK_ENABLE_SECURITY

/*****************************************************************************
*****************************************************************************/
#if SYS_SECURITY_MODE == 1
static void swEncryptReq(uint32_t *text, uint32_t *key);
#endif

/*****************************************************************************
*****************************************************************************/
void SYS_EncryptReq(uint8_t *text, uint8_t *key)
{
#if SYS_SECURITY_MODE == 0
  PHY_EncryptReq(text, key);
#elif SYS_SECURITY_MODE == 1
  swEncryptReq((uint32_t *)text, (uint32_t *)key);
#endif
}

#if SYS_SECURITY_MODE == 0
/*****************************************************************************
*****************************************************************************/
void PHY_EncryptConf(void)
{
  SYS_EncryptConf();
}
#endif

#if SYS_SECURITY_MODE == 1
/*****************************************************************************
*****************************************************************************/
static void xtea(uint32_t text[2], uint32_t const key[4])
{
  uint32_t t0 = text[0];
  uint32_t t1 = text[1];
  uint32_t sum = 0;
  uint32_t delta = 0x9e3779b9;

  for (uint8_t i = 0; i < 32; i++)
  {
    t0 += (((t1 << 4) ^ (t1 >> 5)) + t1) ^ (sum + key[sum & 3]);
    sum += delta;
    t1 += (((t0 << 4) ^ (t0 >> 5)) + t0) ^ (sum + key[(sum >> 11) & 3]);
  }
  text[0] = t0;
  text[1] = t1;
}

/*****************************************************************************
*****************************************************************************/
static void swEncryptReq(uint32_t *text, uint32_t *key)
{
  xtea(&text[0], key);
  text[2] ^= text[0];
  text[3] ^= text[1];
  xtea(&text[2], key);

  SYS_EncryptConf();
}
#endif

#endif // NWK_ENABLE_SECURITY