/**
 * \file halPhy.h
 *
 * \brief ATxmega128b1 PHY interface
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
 * $Id: halPhy.h 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

#ifndef _HAL_PHY_H_
#define _HAL_PHY_H_

/*- Includes ---------------------------------------------------------------*/
#include <stdint.h>
#include "hal.h"
#include "halGpio.h"

/*- Definitions ------------------------------------------------------------*/
#if defined(PLATFORM_XPLAINED)
  HAL_GPIO_PIN(PHY_RST,    C, 0);
  HAL_GPIO_PIN(PHY_IRQ,    C, 2);
  HAL_GPIO_PIN(PHY_SLP_TR, C, 3);
  HAL_GPIO_PIN(PHY_CS,     C, 4);
  HAL_GPIO_PIN(PHY_MOSI,   C, 5);
  HAL_GPIO_PIN(PHY_MISO,   C, 6);
  HAL_GPIO_PIN(PHY_SCK,    C, 7);
#endif

/*- Prototypes -------------------------------------------------------------*/
uint8_t HAL_PhySpiWriteByte(uint8_t value);
void HAL_PhyReset(void);
void halPhyInit(void);

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
INLINE uint8_t HAL_PhySpiWriteByteInline(uint8_t value)
{
  SPIC.DATA = value;
  while (!(SPIC.STATUS & SPI_IF_bm));
  return SPIC.DATA;
}

/*************************************************************************//**
*****************************************************************************/
INLINE void HAL_PhySpiSelect(void)
{
  HAL_GPIO_PHY_CS_clr();
}

/*************************************************************************//**
*****************************************************************************/
INLINE void HAL_PhySpiDeselect(void)
{
  HAL_GPIO_PHY_CS_set();
}

/*************************************************************************//**
*****************************************************************************/
INLINE void HAL_PhySlpTrSet(void)
{
  HAL_GPIO_PHY_SLP_TR_set();
}

/*************************************************************************//**
*****************************************************************************/
INLINE void HAL_PhySlpTrClear(void)
{
  HAL_GPIO_PHY_SLP_TR_clr();
}

#endif // _HAL_PHY_H_

