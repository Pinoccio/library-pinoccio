/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include "../../Shell.h"
#include "SnifferModule.h"

#include <lwm/phy/atmegarfr2.h>
#include "Scout.h"

#include "util/StringBuffer.h"
#include "util/String.h"
#include "util/PrintToString.h"

using namespace pinoccio;

SnifferModule SnifferModule::instance;
static bool binary;

// A queue of packets captured, awaiting 
struct Queue {
  struct Packet {
    uint8_t size; // Total packet size
    uint8_t sent; // How many bytes were sent?
    uint8_t data[NWK_FRAME_MAX_PAYLOAD_SIZE]; // Data
  } *packets;
  uint8_t head; // Index of next packet to enqueue
  uint8_t tail; // Index of next packet to dequeue
  uint8_t len; // Number of packets the queue has space for
};

/****************************\
 *   MODULE CLASS STUFF     *
\****************************/

const __FlashStringHelper *SnifferModule::name() const {
  return F("sniffer");
}

// Calculate the next value for a queue pointer
static uint8_t next(Queue *q, uint8_t n) {
  if (n + 1 == q->len)
    return 0;
  else
    return n + 1;
}

// Read a packet from the hardware buffer into the queue
static void read_packet(Queue *q) {
  uint8_t n = next(q, q->head);

  // Queue full, just drop the packet
  if (n == q->tail)
    return;

  Queue::Packet *p = &q->packets[q->head];

  p->size = TST_RX_LENGTH_REG;
  if (p->size > sizeof(q->packets->data))
    return;

  memcpy(p->data, (const void*)&TRX_FRAME_BUFFER(0), p->size);
  p->sent = 0;
  q->head = n;
}

// Send some data from the queue over serial
static void process_packets(Queue *q) {
  if (q->head == q->tail)
    return;

  Queue::Packet *p = &q->packets[q->tail];

  if (binary && p->sent == 0)
    Serial.write(p->size);

  // Only send a single data packet each time, to prevent blocking on
  // the serial port for too long and missing a packet.
  if (p->size) {
    uint8_t b = p->data[p->sent];
    if (binary) {
      Serial.write(b);
    } else {
      if (b < 0x10)
        Serial.write('0');
      Serial.print(b, HEX);
    }
    p->sent++;
  }

  if (p->sent == p->size) {
    q->tail = next(q, q->tail);

    if (!binary)
      Serial.println();
  }
}

/* Helper functions, taken from the LWM library */
static void phyTrxSetState(uint8_t state)
{
  TRX_STATE_REG = TRX_CMD_FORCE_TRX_OFF;
  while (TRX_STATUS_TRX_OFF != TRX_STATUS_REG_s.trxStatus);

  TRX_STATE_REG = state;
  while (state != TRX_STATUS_REG_s.trxStatus);
}


/**
 * This sets up 'promiscuous' mode, in the sense that it just receives
 * _all_ packets, without checking for validity, addressing or even CRC.
 * This is different from setting the sniffer mode suggested in the
 * datasheet, which works in RX_AACK state with AACK_PROM_MODE and
 * AACK_DIS_AC enabled. That approach still does  CRC checks and
 * possibly also some address filter checks, which we don't want here.
 */
static void setup_promisc() {
  phyTrxSetState(TRX_CMD_RX_ON);
}

static numvar start() {
  if (!checkArgs(0, 1, F("usage: sniffer.start([binary])"))) {
    return 0;
  }

  // Allocate a queue of packet payloads as big as possible - we won't
  // need any more memory after this (malloc reserves a bit of memory
  // for the stack which is more than enough for the few function calls
  // and ISRs we've left to do).
  Queue q = {
    .packets = NULL,
    .head = 0,
    .tail = 0,
    .len = 0,
  };
  while(q.len < 256 /* head and tail are uin8_t */) {
    void* newp = realloc(q.packets, (q.len + 1) * sizeof(*q.packets));
    // No more memory, done
    if (!newp)
      break;
    q.packets = (Queue::Packet*)newp;
    q.len++;
  }

  binary = getarg(0) ? getarg(1) : 0;

  if (!binary) {
    Serial.println(F("Sniffing mesh traffic and output packets to serial. While"));
    Serial.println(F("sniffing, the scout does not respond to anything else (mesh,"));
    Serial.println(F("serial, wifi, etc.) and does not run any ScoutScript. The"));
    Serial.println(F("only way to stop sniffing is to reset."));
    Serial.print(F("Buffer size is "));
    Serial.print(q.len);
    Serial.println(F(" packets."));
  } else {
    // Magic string for syncing the stream. Insired by
    // http://cetic.github.io/foren6/guide.html and
    // https://github.com/cetic/contiki/tree/sniffer/examples/sniffer
    // though we do not use the same enable sequence (yet?)
    Serial.write("SNIF");
  }

  setup_promisc();

  while(true) {
    if (IRQ_STATUS_REG_s.rxEnd)
    {
      read_packet(&q);
      while (TRX_STATUS_RX_ON != TRX_STATUS_REG_s.trxStatus);

      IRQ_STATUS_REG_s.rxEnd = 1;
      TRX_CTRL_2_REG_s.rxSafeMode = 0;
      TRX_CTRL_2_REG_s.rxSafeMode = 1;
    }
    process_packets(&q);
  }
  // Not reached
  return 1;
}

static numvar ed() {
  return PHY_EdReq();
}

static numvar survey() {
  StringBuffer out(200);

  for (uint8_t i = 11; i <= 26; i++){
    PHY_SetChannel(i);

    int8_t max = -90;
    for (uint8_t j = 0; j <= 254; j++){
      int8_t reading = PHY_EdReq();
      if(max < reading){
        max = reading;
      }
    }

    out.appendSprintf("Ch: %d\t%d\r\n", i, max);
  }

  PHY_SetChannel(Scout.getChannel());
  speol(out.c_str());
  return 1;
}

bool SnifferModule::enable() {
  Shell.addFunction("sniffer.start", start);
  Shell.addFunction("sniffer.ed", ed);
  Shell.addFunction("sniffer.survey", survey);

  return true;
}

void SnifferModule::loop() {
}
