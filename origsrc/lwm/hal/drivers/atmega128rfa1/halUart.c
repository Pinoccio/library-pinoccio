/**
 * \file halUart.c
 *
 * \brief ATmega128rfa1 UART implementation
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
 * $Id: halUart.c 7863 2013-05-13 20:14:34Z ataradov $
 *
 */

/*- Includes ---------------------------------------------------------------*/
#include <stdbool.h>
#include "hal.h"
#include "halUart.h"
#include "config.h"

/*- Definitions ------------------------------------------------------------*/
#ifndef HAL_UART_TX_FIFO_SIZE
#define HAL_UART_TX_FIFO_SIZE  10
#endif

#ifndef HAL_UART_RX_FIFO_SIZE
#define HAL_UART_RX_FIFO_SIZE  10
#endif

#ifndef HAL_UART_CHANNEL
#define HAL_UART_CHANNEL       0
#endif

#if HAL_UART_CHANNEL == 0
  #define UBRRxH            UBRR0H
  #define UBRRxL            UBRR0L
  #define UCSRxA            UCSR0A
  #define UCSRxB            UCSR0B
  #define UCSRxC            UCSR0C
  #define UDRx              UDR0
  #define USARTx_UDRE_vect  USART0_UDRE_vect
  #define USARTx_RX_vect    USART0_RX_vect
#elif HAL_UART_CHANNEL == 1
  #define UBRRxH            UBRR1H
  #define UBRRxL            UBRR1L
  #define UCSRxA            UCSR1A
  #define UCSRxB            UCSR1B
  #define UCSRxC            UCSR1C
  #define UDRx              UDR1
  #define USARTx_UDRE_vect  USART1_UDRE_vect
  #define USARTx_RX_vect    USART1_RX_vect
#else
  #error Unsupported UART channel
#endif

/*- Types ------------------------------------------------------------------*/
typedef struct
{
  uint16_t  head;
  uint16_t  tail;
  uint16_t  size;
  uint16_t  bytes;
  uint8_t   *data;
} FifoBuffer_t;

/*- Variables --------------------------------------------------------------*/
static FifoBuffer_t txFifo;
static uint8_t txData[HAL_UART_TX_FIFO_SIZE+1];

static volatile FifoBuffer_t rxFifo;
static uint8_t rxData[HAL_UART_RX_FIFO_SIZE+1];

static volatile bool udrEmpty;
static volatile bool newData;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
void HAL_UartInit(uint32_t baudrate)
{
  uint32_t brr = ((uint32_t)F_CPU * 2) / (16 * baudrate) - 1;

  UBRRxH = (brr >> 8) & 0xff;
  UBRRxL = (brr & 0xff);
  UCSRxA = (1 << U2X1);
  UCSRxB = (1 << TXEN1) | (1 << RXEN1) | (1 << RXCIE1);
  UCSRxC = (3 << UCSZ10);

  txFifo.data = txData;
  txFifo.size = HAL_UART_TX_FIFO_SIZE;
  txFifo.bytes = 0;
  txFifo.head = 0;
  txFifo.tail = 0;

  rxFifo.data = rxData;
  rxFifo.size = HAL_UART_RX_FIFO_SIZE;
  rxFifo.bytes = 0;
  rxFifo.head = 0;
  rxFifo.tail = 0;

  udrEmpty = true;
  newData = false;
}

/*************************************************************************//**
*****************************************************************************/
void HAL_UartWriteByte(uint8_t byte)
{
  if (txFifo.bytes == txFifo.size)
    return;

  txFifo.data[txFifo.tail++] = byte;
  if (txFifo.tail == txFifo.size)
    txFifo.tail = 0;
  txFifo.bytes++;
}

/*************************************************************************//**
*****************************************************************************/
uint8_t HAL_UartReadByte(void)
{
  uint8_t byte;

  PRAGMA(diag_suppress=Pa082);
  ATOMIC_SECTION_ENTER
    byte = rxFifo.data[rxFifo.head++];
    if (rxFifo.head == rxFifo.size)
      rxFifo.head = 0;
    rxFifo.bytes--;
  ATOMIC_SECTION_LEAVE
  PRAGMA(diag_default=Pa082);

  return byte;
}

/*************************************************************************//**
*****************************************************************************/
ISR(USARTx_UDRE_vect)
{
  udrEmpty = true;
  UCSRxB &= ~(1 << UDRIE1);
}

/*************************************************************************//**
*****************************************************************************/
ISR(USARTx_RX_vect)
{
  PRAGMA(diag_suppress=Pa082);

  uint8_t status = UCSRxA;
  uint8_t byte = UDRx;

  if (0 == (status & ((1 << FE1) | (1 << DOR1) | (1 << UPE1))))
  {
    if (rxFifo.bytes == rxFifo.size)
      return;

    rxFifo.data[rxFifo.tail++] = byte;
    if (rxFifo.tail == rxFifo.size)
      rxFifo.tail = 0;
    rxFifo.bytes++;

    newData = true;
  }

  PRAGMA(diag_default=Pa082);
}

/*************************************************************************//**
*****************************************************************************/
void HAL_UartTaskHandler(void)
{
  if (txFifo.bytes && udrEmpty)
  {
    uint8_t byte;

    byte = txFifo.data[txFifo.head++];
    if (txFifo.head == txFifo.size)
      txFifo.head = 0;
    txFifo.bytes--;

    ATOMIC_SECTION_ENTER
      UDRx = byte;
      UCSRxB |= (1 << UDRIE1);
      udrEmpty = false;
    ATOMIC_SECTION_LEAVE
  }

  {
    uint16_t bytes;
    bool new;

    ATOMIC_SECTION_ENTER
      new = newData;
      newData = false;
      bytes = rxFifo.bytes;
    ATOMIC_SECTION_LEAVE

    if (new)
      HAL_UartBytesReceived(bytes);
  }
}
