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

#define MOVAVG_SIZE 8

namespace pinoccio {
  class EnvModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static EnvModule instance;

      void tempPushMovingAvg(float val);
      float tempGetMovingAvgC();
      float tempGetMovingAvgF();

      void humidityPushMovingAvg(float val);
      float humidityGetMovingAvg();
      
      void baroPushMovingAvg(float val);   
      float baroGetMovingAvgKpa();
      float baroGetMovingAvgMbar();
      float baroGetMovingAvgMmhg();
      float baroGetMovingAvgInhg();

      uint16_t lightGetIr();
      uint16_t lightGetVisible();
      uint16_t lightGetFull();
      uint16_t lightGetLux();
      
      bool lightConfigure(uint8_t sensitivity);
      
      HTU21D *htu;
      Adafruit_MPL115A2 *mpl;
      TSL2561 *tsl;
      
    private:
      float *tempMovingAvgBuffer;
      int tempMovingAvgCtr;
      
      float *humidityMovingAvgBuffer;
      int humidityMovingAvgCtr;
      
      float *baroMovingAvgBuffer;
      int baroMovingAvgCtr;
    
      SYS_Timer_t htuMovingAvgTimer;
      SYS_Timer_t baroMovingAvgTimer;
      
      using Module::Module;
  };
} // namespace pinoccio
#endif