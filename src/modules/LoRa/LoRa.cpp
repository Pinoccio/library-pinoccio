/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Scout.h>
#include "LoRa.h"

#include "SX1272.h"

using namespace pinoccio;

LoRaModule LoRaModule::instance;

const __FlashStringHelper *LoRaModule::name() const {
  return F("lora");
}

// any scoutscript commands
static numvar ping() {
  if (!checkArgs(1, F("usage: lora.ping(id)"))) {
    return 0;
  }

  int err = sx1272.sendPacketTimeoutACK(getarg(1), "test", 50);
  if(err)
  {
    speol("ping failed");
  }else{
    speol("ping success");
  }
  return err;
}

bool LoRaModule::enable() {
  int err;
  // Power ON the module
  sx1272.ON();
  
  // Set transmission mode
  err = sx1272.setMode(4);
  
  // Select frequency channel
  err += sx1272.setChannel(CH_10_868);
  
  // Select output power (Max, High or Low)
  err += sx1272.setPower('H');
  
  // Set the node address and print the result
  err += sx1272.setNodeAddress(Scout.getAddress());
  if(err)
  {
    sp("LoRa SX1272 init failed: ");
    speol(err);
  }else{
    speol("LoRa SX1272 successfully configured");
  }
  
  Shell.addFunction("lora.ping", ping);
}

void LoRaModule::loop() {
  sx1272.receiveAll(10); // just testing
}

