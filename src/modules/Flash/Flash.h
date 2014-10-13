/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_SERIAL_FLASH_MODULE_H_
#define LIB_PINOCCIO_SERIAL_FLASH_MODULE_H_

#include "../../peripherals/Flash.h"
#define SERIAL_FLASH_BUFSIZE 65

#include "../Module.h"

namespace pinoccio {
  class FlashModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static FlashModule instance;

      uint8_t csPin;    
      FlashClass *flash;

    private:
      using Module::Module;
  };
}

#endif