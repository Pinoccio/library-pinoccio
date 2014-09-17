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
#include "Env.h"

using namespace pinoccio;

EnvModule EnvModule::instance;

const __FlashStringHelper *EnvModule::name() const {
  return F("env");
}

// any scoutscript commands
static numvar status() {
  EnvModule* m = &(EnvModule::instance);
  sp("Temp C  : ");
  speol(m->tempGetMovingAvgC(), 1);
  sp("Temp F  : ");
  speol(m->tempGetMovingAvgF(), 1);
  sp("Humidity: ");
  speol(m->humidityGetMovingAvg(), 1);
}

static numvar getHumidity() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->humidityGetMovingAvg());
}

static numvar getTemperatureC() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->tempGetMovingAvgC());
}

static numvar getTemperatureF() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->tempGetMovingAvgF());
}

static void htuMovingAvgTimerHandler(SYS_Timer_t *timer) {
  EnvModule* m = &(EnvModule::instance);
  m->humidityPushMovingAvg(m->htu->readHumidity());
  m->tempPushMovingAvg(m->htu->readTemperature());
}

bool EnvModule::enable() {
  htu = new HTU21D();
  tempMovingAvgBuffer = new float[MOVAVG_SIZE];
  humidityMovingAvgBuffer = new float[MOVAVG_SIZE];

  Shell.addFunction("env.f", getTemperatureF);
  Shell.addFunction("env.c", getTemperatureC);
  Shell.addFunction("env.humidity", getHumidity);
  Shell.addFunction("env.status", status);
  Serial.println("env enabled");

  for(int i=0; i<MOVAVG_SIZE; i++) {
    humidityPushMovingAvg(htu->readHumidity());
    tempPushMovingAvg(htu->readTemperature());
  }

  htuMovingAvgTimer.interval = 250;
  htuMovingAvgTimer.mode = SYS_TIMER_PERIODIC_MODE;
  htuMovingAvgTimer.handler = htuMovingAvgTimerHandler;
  SYS_TimerStart(&htuMovingAvgTimer);
}

void EnvModule::loop() { }

float EnvModule::humidityGetMovingAvg(void) {
  float sum = 0.0;
  for(int i=0; i<MOVAVG_SIZE; i++) {
    sum += humidityMovingAvgBuffer[i];
  }
  return sum / MOVAVG_SIZE;
}

void EnvModule::humidityPushMovingAvg(float val) {
  humidityMovingAvgBuffer[humidityMovingAvgCtr] = val;
  humidityMovingAvgCtr = (humidityMovingAvgCtr + 1) % MOVAVG_SIZE;
}

float EnvModule::tempGetMovingAvgC(void) {
  float sum = 0.0;
  for(int i=0; i<MOVAVG_SIZE; i++) {
    sum += tempMovingAvgBuffer[i];
  }
  return sum / MOVAVG_SIZE;
}

float EnvModule::tempGetMovingAvgF(void) {
  return round((1.8 * tempGetMovingAvgC()) + 32);
}

void EnvModule::tempPushMovingAvg(float val) {
  tempMovingAvgBuffer[tempMovingAvgCtr] = val;
  tempMovingAvgCtr = (tempMovingAvgCtr + 1) % MOVAVG_SIZE;
}

