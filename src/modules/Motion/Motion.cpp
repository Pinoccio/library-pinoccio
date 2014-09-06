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
#include "modules/Motion/Motion.h"
#include "modules/Motion/Adafruit_GPS.h"
//#include "modules/Motion/MPU9150.h"
#include "modules/Motion/MS561101BA.h"

using namespace pinoccio;

MotionModule MotionModule::instance;

const __FlashStringHelper *MotionModule::name() const {
  return F("motion");
}

static void gpsTimerHandler(SYS_Timer_t *timer) {
  MotionModule* m = &(MotionModule::instance);
  m->gps->read();
  if (m->gps->newNMEAreceived()) {
    m->gps->parse(m->gps->lastNMEA());
  }
}

static numvar verbose(void) {
  if (!checkArgs(1, F("usage: motion.verbose(flag)"))) {
    return 0;
  }
  MotionModule* m = &(MotionModule::instance);
  m->isVerbose = getarg(1);
}
  
static numvar gpsTime(void) {
  MotionModule* m = &(MotionModule::instance);
  if (m->gps->fix == 0) {
    speol("No satellite fix. Time not available");
    return 0;
  }
  sp("20");
  sp(m->gps->year);
  sp("-");
  sp(m->gps->month);
  sp("-");
  sp(m->gps->day);
  sp(" ");
  sp(m->gps->hour);
  sp(":");
  sp(m->gps->minute);
  sp(":");
  sp(m->gps->seconds);
  sp("/");
  speol(m->gps->milliseconds);
  return 1;
}

static numvar gpsSatFix(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->fix;
}

static numvar gpsSatFixQuality(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->fixquality;
}

static numvar gpsSatCount(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->satellites;
}

static numvar gpsLatitude(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->lat;
}

static numvar gpsLongitude(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->lon;
}

static numvar gpsSpeed(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->speed;
}

static numvar gpsAngle(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->gps->angle;
}


static void baroMovingAvgTimerHandler(SYS_Timer_t *timer) {
  MotionModule* m = &(MotionModule::instance);
  m->msPushMovingAvg(m->ms->getPressure(MS561101BA_OSR_4096));
}

static numvar baroPressure(void) {
  MotionModule* m = &(MotionModule::instance);
  sp(m->msGetPressure(),2);
  speol();
  return m->msGetPressure();
}

static numvar baroAltitude(void) {
  MotionModule* m = &(MotionModule::instance);

  return ((pow((1013.25 / m->msGetPressure()), 1/5.257) - 1.0) * (m->ms->getTemperature(MS561101BA_OSR_4096) + 273.15)) / 0.0065;
}

static numvar baroTemperature(void) {
  MotionModule* m = &(MotionModule::instance);
  return m->ms->getTemperature(MS561101BA_OSR_4096);
}

bool MotionModule::enable() {
  gps = new Adafruit_GPS();
  //mpu = new MPU9150();
  ms = new MS561101BA();

  msMovingAvgCtr = 0;
  isVerbose = false;

  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  
  Shell.addFunction("motion.verbose", verbose);
  
  Shell.addFunction("motion.gps.time", gpsTime);
  Shell.addFunction("motion.gps.sat.fix", gpsSatFix);
  Shell.addFunction("motion.gps.sat.fixquality", gpsSatFixQuality);
  Shell.addFunction("motion.gps.sat.count", gpsSatCount);
  Shell.addFunction("motion.gps.latitude", gpsLatitude);
  Shell.addFunction("motion.gps.longitude", gpsLongitude);
  Shell.addFunction("motion.gps.speed", gpsSpeed);
  Shell.addFunction("motion.gps.angle", gpsAngle);
  /*
  Shell.addFunction("motion.mpu.gyro.yaw", mpuGyroYaw);
  Shell.addFunction("motion.mpu.gyro.pitch", mpuGyroPitch);
  Shell.addFunction("motion.mpu.gyro.roll", mpuGyroRoll);
  Shell.addFunction("motion.mpu.accel.x", mpuAccelX);
  Shell.addFunction("motion.mpu.accel.y", mpuAccelY);
  Shell.addFunction("motion.mpu.accel.z", mpuAccelZ);
  Shell.addFunction("motion.mpu.mag.heading", mpuMagHeading);
  */
  Shell.addFunction("motion.baro.pressure", baroPressure);
  Shell.addFunction("motion.baro.altitude", baroAltitude);
  Shell.addFunction("motion.baro.temperature", baroTemperature);

  gps->begin(9600);
  //gps->sendCommand(PMTK_SET_BAUD_115200);
  //gps->begin(115200);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  gps->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //gps->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  gps->sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  //Serial1.println(PMTK_Q_RELEASE);
  
  gpsTimer.interval = 50;
  gpsTimer.mode = SYS_TIMER_PERIODIC_MODE;
  gpsTimer.handler = gpsTimerHandler;
  SYS_TimerStart(&gpsTimer);

  //mpu->initialize();
  //speol(mpu.testConnection() ? F("MPU9150 connection successful") : F("MPU9150 not found"));

  ms->init(MS561101BA_ADDR_CSB_LOW);
  delay(100);
  ms->getTemperature(MS561101BA_OSR_4096); // need an initial getTemperature() to get started for some reason
  for(int i=0; i<MOVAVG_SIZE; i++) {
    msMovingAvgBuffer[i] = ms->getPressure(MS561101BA_OSR_4096);
  }
    
  baroMovingAvgTimer.interval = 50;
  baroMovingAvgTimer.mode = SYS_TIMER_PERIODIC_MODE;
  baroMovingAvgTimer.handler = baroMovingAvgTimerHandler;
  SYS_TimerStart(&baroMovingAvgTimer);
}

void MotionModule::loop() {
  if (isVerbose) {
    while (Serial1.available()) {
      Serial.write(Serial1.read());
    }
  }
}

float MotionModule::msGetPressure(void) {
  float sum = 0.0;
  for(int i=0; i<MOVAVG_SIZE; i++) {
    sum += msMovingAvgBuffer[i];
  }
  return sum / MOVAVG_SIZE;
}

void MotionModule::msPushMovingAvg(float val) {
  msMovingAvgBuffer[msMovingAvgCtr] = val;
  msMovingAvgCtr = (msMovingAvgCtr + 1) % MOVAVG_SIZE;
}
