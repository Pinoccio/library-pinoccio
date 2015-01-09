/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_ENV_MODULE_H_
#define LIB_PINOCCIO_ENV_MODULE_H_

#include "../Module.h"
#include "HTU21D.h"
#include "Adafruit_MPL115A2.h"
#include "TSL2561.h"

#define MOVAVG_SIZE 5

namespace pinoccio {
  class EnvModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static EnvModule instance;

      float tempGetC();
      float tempGetF();

      float humidityGetPercent();
      
      float baroGetKpa();
      float baroGetMbar();
      float baroGetMmHg();
      float baroGetInHg();

      uint16_t lightGetIr();
      uint16_t lightGetVisible();
      uint16_t lightGetFull();
      uint16_t lightGetLux();
      
      bool lightConfigure(uint8_t sensitivity);
      
      Adafruit_HTU21DF *htu;
      Adafruit_MPL115A2 *mpl;
      TSL2561 *tsl;
      
    private:
      using Module::Module;
  };
} // namespace pinoccio
#endif
