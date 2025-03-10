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
#include "BLE.h"
#include "util/PrintToString.h"

using namespace pinoccio;

BLEModule BLEModule::instance;

const __FlashStringHelper *BLEModule::name() const {
  return F("ble");
}

static numvar blePrintLn() {
  BLEModule* m = &(BLEModule::instance);
  return m->ble->println((const char *)getstringarg(1));
}

// TODO: bitlash function to handle reads via BLE_UART callback

bool BLEModule::enable() {
  lastStatus = ACI_EVT_DISCONNECTED;
  ble = new Adafruit_BLE_UART(SS, 4, 8);
  ble->begin();

  Shell.addFunction("ble.println", blePrintLn);
  Serial.println("ble enabled");
}


StringBuffer bleOutput;

void BLEModule::loop() {
  ble->pollACI();

  // Ask what is our current status
  aci_evt_opcode_t status = ble->getState();
  // If the status changed....
  if (status != lastStatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
      speol(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
      speol(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
      speol(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    lastStatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {
    // Lets see if there's any data for us!
    StringBuffer bin(20);
    bin = "";
    while (ble->available()) {
      bin += (char)ble->read();
    }

    if (bin.length()) {
      Serial.println(bin.c_str());
      setOutputHandler(&printToString<&bleOutput>);
      doCommand(const_cast<char *>(bin.c_str()));
      resetOutputHandler();

      Serial.println(bleOutput.c_str());
      ble->write((uint8_t*)bleOutput.c_str(), bleOutput.length());
      bleOutput = "";
    }
  }
}
