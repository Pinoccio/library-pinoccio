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

using namespace pinoccio;

SnifferModule SnifferModule::instance;
static bool binary;

/****************************\
 *   MODULE CLASS STUFF     *
\****************************/

const __FlashStringHelper *SnifferModule::name() const {
  return F("sniffer");
}

static void process_packet() {
  uint8_t size = TST_RX_LENGTH_REG;

  if (binary)
    Serial.write(size);

  for (uint8_t i = 0; i < size; i++) {
    uint8_t b = TRX_FRAME_BUFFER(i);
    if (binary) {
      Serial.write(b);
    } else {
      if (b < 0x10)
        Serial.write('0');
      Serial.print(b, HEX);
    }
  }
  if (!binary)
    Serial.println();
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

  binary = getarg(0) ? getarg(1) : 0;

  if (!binary) {
    Serial.println(F("Sniffing mesh traffic and output packets to serial. While"));
    Serial.println(F("sniffing, the scout does not respond to anything else (mesh,"));
    Serial.println(F("serial, wifi, etc.) and does not run any ScoutScript. The"));
    Serial.println(F("only way to stop sniffing is to reset."));
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
      process_packet();
      while (TRX_STATUS_RX_ON != TRX_STATUS_REG_s.trxStatus);

      IRQ_STATUS_REG_s.rxEnd = 1;
      TRX_CTRL_2_REG_s.rxSafeMode = 0;
      TRX_CTRL_2_REG_s.rxSafeMode = 1;
    }
  }
  // Not reached
  return 1;
}

bool SnifferModule::enable() {
  Shell.addFunction("sniffer.start", start);

  return true;
}

void SnifferModule::loop() {
}
