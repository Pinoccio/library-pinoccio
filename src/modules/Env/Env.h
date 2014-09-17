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

#define MOVAVG_SIZE 8

namespace pinoccio {
  class EnvModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static EnvModule instance;

      void humidityPushMovingAvg(float val);
      float humidityGetMovingAvg();
      
      void tempPushMovingAvg(float val);
      float tempGetMovingAvgC();
      float tempGetMovingAvgF();

      HTU21D *htu;
    private:
      float *tempMovingAvgBuffer;
      int tempMovingAvgCtr;
      
      float *humidityMovingAvgBuffer;
      int humidityMovingAvgCtr;
    
      SYS_Timer_t htuMovingAvgTimer;
      
      using Module::Module;
  };
} // namespace pinoccio
#endif