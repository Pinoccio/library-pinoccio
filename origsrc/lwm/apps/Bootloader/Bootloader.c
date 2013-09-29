/**
 * \file bootloader.c
 *
 * \brief Bootloader implementation
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
 * $Id: Bootloader.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include "sysTypes.h"

/*- Definitions ------------------------------------------------------------*/
#define XMODEM_BUFFER_SIZE     128

#if defined(HAL_ATMEGA128RFA1)
  #define BOOT_PORT   PORTE
  #define BOOT_PIN    PINE
  #define BOOT_INDEX  5

  #define ENABLE_RS232_CONVERTER

#elif defined(HAL_ATMEGA1281)
  #define BOOT_PORT   PORTE
  #define BOOT_PIN    PINE
  #define BOOT_INDEX  6

#else
  #error Unknown HAL
#endif

/*- Types ------------------------------------------------------------------*/
enum
{
  XMODEM_SOH = 1,
  XMODEM_EOT = 4,
  XMODEM_ACK = 6,
  XMODEM_NAK = 21,
};

enum
{
  IAP_WRITE_PAGE = 0x00,
  IAP_SWITCH     = 0x01,
};

typedef struct iap_request_t
{
  uint8_t   id;
  uint32_t  addr;
  uint16_t  page;
} iap_request_t;

/*- Prototypes -------------------------------------------------------------*/
void iap_write_page_handler(uint32_t addr, uint16_t *buf);
void iap_switch_handler(void);
static void write_page(uint32_t addr, uint16_t *buf);
static void reboot(void);

/*- Variables --------------------------------------------------------------*/
__attribute__((section(".iap.write_page"))) void (*iap_write_page)(uint32_t, uint16_t *) = iap_write_page_handler;
__attribute__((section(".iap.switch"))) void (*iap_switch)(void) = iap_switch_handler;
static uint8_t buf[1/*SOH*/ + 1/*seq*/ + 1/*nseq*/ + XMODEM_BUFFER_SIZE + 2/*crc*/];
static uint8_t page[SYS_PAGE_SIZE];
static uint32_t addr = 0;
static uint16_t size = 0;
static uint16_t timeout = 0;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void iap_write_page_handler(uint32_t addr, uint16_t *buf)
{
  write_page(addr, buf);
}

/*************************************************************************//**
*****************************************************************************/
void iap_switch_handler(void)
{
  uint32_t src = SYS_DEVICE_SIZE / 2;
  uint32_t dst = 0;
  uint32_t size = SYS_DEVICE_SIZE / 2 - SYS_BOOTLOADER_SIZE;

  while (size)
  {
    for (uint16_t i = 0; i < sizeof(page); i++)
      page[i] = pgm_read_byte_far(src++);

    write_page(dst, (uint16_t *)page);
    dst += sizeof(page);
    size -= sizeof(page);
  };

  reboot();
}

/*************************************************************************//**
*****************************************************************************/
static void write_page(uint32_t addr, uint16_t *buf)
{
  wdt_reset();

  boot_page_erase(addr);
  boot_spm_busy_wait();

  for (uint16_t i = 0; i < SYS_PAGE_SIZE; i += 2)
    boot_page_fill(addr + i, *buf++);

  boot_page_write(addr);
  boot_spm_busy_wait();

  boot_rww_enable();
}

/*************************************************************************//**
*****************************************************************************/
static void reboot(void)
{
  wdt_reset();
  WDTCSR = (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDE);
  while (1);
}

/*************************************************************************//**
*****************************************************************************/
static void hardware_init(void)
{
  OCR4A = ((F_CPU / 1000ul) / 8);
  TCCR4B = (1 << WGM12) | (1 << CS11); // CTC mode, Prescaler 8

#ifdef ENABLE_RS232_CONVERTER
  DDRD = (1 << 4) | (1 << 6) | (1 << 7);
  PORTD = (0 << 4) | (1 << 6) | (1 << 7);
#endif

  UBRR1H = 0;
  UBRR1L = 25;
  UCSR1A = (1 << U2X1);
  UCSR1B = (1 << TXEN1) | (1 << RXEN1);
  UCSR1C = (3 << UCSZ10);
}

/*************************************************************************//**
*****************************************************************************/
static bool timer_poll(void)
{
  wdt_reset();

  if (timeout && (TIFR4 & (1 << OCF4A)))
  {
    TIFR4 |= (1 << OCF4A);
    timeout--;
  }
  return timeout > 0;
}

/*************************************************************************//**
*****************************************************************************/
static void handle_buffer(void)
{
  for (uint8_t i = 0; i < XMODEM_BUFFER_SIZE; i++)
    page[size + i] = buf[i+3];
  size += XMODEM_BUFFER_SIZE;

  if (SYS_PAGE_SIZE == size)
  {
    write_page(addr, (uint16_t *)page);
    addr += SYS_PAGE_SIZE;
    size = 0;
  }
}

/*************************************************************************//**
*****************************************************************************/
static uint16_t get_crc(void)
{
  uint16_t crc = 0;

  for (uint8_t j = 0; j < 128; j++)
  {
    crc = crc ^ (uint16_t)buf[j+3] << 8;

    for (uint8_t i = 0; i < 8; i++)
    {
      if (crc & 0x8000)
        crc = crc << 1 ^ 0x1021;
      else
        crc = crc << 1;
    }
  }

  return crc;
}

/*************************************************************************//**
*****************************************************************************/
static void send(uint8_t data)
{
  while (!(UCSR1A & (1 << UDRE1)));
  UDR1 = data;
}

/*************************************************************************//**
*****************************************************************************/
static bool recv(uint8_t *data)
{
  uint8_t status = UCSR1A;

  if (status & (1 << RXC1))
  {
    *data = UDR1;

    if (0 == (status & ((1 << FE1) | (1 << DOR1) | (1 << UPE1))))
      return true;
  }

  return false;
}

/*************************************************************************//**
*****************************************************************************/
static void flush(void)
{
  timeout = 1000;
  while (timer_poll())
  {
    uint8_t data;
    recv(&data);
  }
}

/*************************************************************************//**
*****************************************************************************/
static void receive_file(void)
{
  uint8_t data, ptr;
  uint16_t seq = 1;
  uint16_t crc;

  flush();

  while (1)
  {
    if (1 == seq)
      send('C');

    ptr = 0;
    for (uint8_t i = 0; i < sizeof(buf); i++)
      buf[i] = 0xff;

    timeout = 1000;
    while (timer_poll())
    {
      if (recv(&data))
        buf[ptr++] = data;

      if (ptr == sizeof(buf))
        timeout = 0;

      if (XMODEM_EOT == buf[0])
      {
        timeout = 0;
        handle_buffer();
        send(XMODEM_ACK);
        return;
      }
    }

    if (0 == ptr)
      continue;

    crc = get_crc();

    if (XMODEM_SOH == buf[0] && (uint8_t)seq == buf[1] && 0xff == (buf[1] + buf[2]) &&
        (buf[131] == (uint8_t)(crc >> 8)) && (buf[132] == (uint8_t)(crc)))
    {
      handle_buffer();
      seq++;
      send(XMODEM_ACK);
    }
    else
    {
      flush();
      send(XMODEM_NAK);
    }
  }
}

/*************************************************************************//**
*****************************************************************************/
static bool bootloader_mode(void)
{
  bool bootloader = false;

  BOOT_PORT = (1 << BOOT_INDEX);
  for (uint8_t i = 0; i < 100; i++)
    asm("nop");
  bootloader = (0 == (BOOT_PIN & (1 << BOOT_INDEX)));
  BOOT_PORT = 0;

  return bootloader;
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
  void (*reset_vector)(void) = 0;

  if (bootloader_mode())
  {
    hardware_init();
    receive_file();
    reboot();
  }

  reset_vector();
}
