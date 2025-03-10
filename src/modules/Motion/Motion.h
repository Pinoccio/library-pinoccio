/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_MOTION_MODULE_H_
#define LIB_PINOCCIO_MOTION_MODULE_H_

#include "Adafruit_GPS.h"
//#include "MPU9150.h"
#include "MS561101BA.h"

#define MOVAVG_SIZE 32

#include "../Module.h"

namespace pinoccio {
  class MotionModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static MotionModule instance;

      float msGetPressure(void);
      void msPushMovingAvg(float val);
      float msGetMovingAvg();
    
      Adafruit_GPS *gps;  // GPS
      //MPU9150 *mpu;       // MPU9150 gyro/accel/mag
      MS561101BA *ms;     // altimeter
    
      bool isVerbose;

    protected:
    
      float msMovingAvgBuffer[MOVAVG_SIZE];
      int msMovingAvgCtr;
    
      SYS_Timer_t gpsTimer;
      SYS_Timer_t baroMovingAvgTimer;

    private:
      using Module::Module;
  };
}

#endif