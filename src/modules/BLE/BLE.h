/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_BLE_MODULE_H_
#define LIB_PINOCCIO_BLE_MODULE_H_

#include "../Module.h"
#include "Adafruit_BLE_UART.h"

#define ADAFRUITBLE_REQ SS
#define ADAFRUITBLE_RDY 4     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 8

namespace pinoccio {
  class BLEModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static BLEModule instance;

      Adafruit_BLE_UART *ble;
      aci_evt_opcode_t lastStatus;

    private:
      using Module::Module;
  };
} // namespace pinoccio
#endif