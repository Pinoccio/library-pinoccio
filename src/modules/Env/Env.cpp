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
  sp("Pressure kPa: ");
  speol(m->baroGetMovingAvgKpa(), 1);
  sp("Pressure mbar: ");
  speol(m->baroGetMovingAvgMbar(), 1);
  sp("Pressure mmHg: ");
  speol(m->baroGetMovingAvgMmhg(), 1);
  sp("Pressure mmIn: ");
  speol(m->baroGetMovingAvgInhg(), 1);
  sp("Light IR: ");
  speol(m->lightGetIr());
  sp("Light Visible: ");
  speol(m->lightGetVisible());
  sp("Light Full: ");
  speol(m->lightGetFull());
  sp("Light Lux: ");
  speol(m->lightGetLux());
}

static numvar getTemperatureC() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->tempGetMovingAvgC());
  return (int)round(m->htu->readTemperature());
}

static numvar getTemperatureF() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->tempGetMovingAvgF());
  return  (int)round((1.8 * m->htu->readTemperature()) + 32);
}

static numvar getHumidity() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->humidityGetMovingAvg());
  return (int)round(m->htu->readHumidity());
}

static numvar getPressureKpa() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->baroGetMovingAvgKpa());
  return (int)round(m->mpl->getPressure());
}

static numvar getPressureMbar() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->baroGetMovingAvgMbar());
  return (int)round(m->mpl->getPressure() * 10);
}

static numvar getPressureMmhg() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->baroGetMovingAvgMmhg());
  return (int)round(m->mpl->getPressure() * 7.50062);
}

static numvar getPressureInhg() {
  EnvModule* m = &(EnvModule::instance);
  //return (int)round(m->baroGetMovingAvgInhg());
  return (int)round(m->mpl->getPressure() * 0.2953);
}

static numvar getLightIr() {
  EnvModule* m = &(EnvModule::instance);
  return m->lightGetIr();
}

static numvar getLightVisible() {
  EnvModule* m = &(EnvModule::instance);
  return m->lightGetVisible();
}

static numvar getLightFull() {
  EnvModule* m = &(EnvModule::instance);
  return m->lightGetFull();
}

static numvar getLightLux() {
  EnvModule* m = &(EnvModule::instance);
  return m->lightGetLux();
}

static numvar lightConfig() {
  EnvModule* m = &(EnvModule::instance);
  return m->lightConfigure(getarg(1));
}

static void htuMovingAvgTimerHandler(SYS_Timer_t *timer) {
  EnvModule* m = &(EnvModule::instance);
  m->cc2->triggerMeasurement();
  delay(50);
  m->humidityPushMovingAvg(m->cc2->readHumidity());
  m->tempPushMovingAvg(m->cc2->readTemperature());
}

static void baroMovingAvgTimerHandler(SYS_Timer_t *timer) {
  EnvModule* m = &(EnvModule::instance);
  m->baroPushMovingAvg(m->mpl->getPressure());
}

bool EnvModule::enable() {
  htu = new Adafruit_HTU21DF();
  htu->begin();
  
  /*
  cc2 = new ChipCap2();
  if (cc2->present()) {
    cc2->triggerMeasurement();
  }
  */
  
  mpl = new Adafruit_MPL115A2();
  mpl->begin();
  
  tsl = new TSL2561(TSL2561_ADDR_FLOAT);
  tsl->begin();
  
  tempMovingAvgBuffer = new float[MOVAVG_SIZE];
  humidityMovingAvgBuffer = new float[MOVAVG_SIZE];
  baroMovingAvgBuffer = new float[MOVAVG_SIZE];

  Shell.addFunction("env.temp.f", getTemperatureF);
  Shell.addFunction("env.temp.c", getTemperatureC);
  Shell.addFunction("env.humidity", getHumidity);
  Shell.addFunction("env.baro.kpa", getPressureKpa);
  Shell.addFunction("env.baro.mbar", getPressureMbar);
  Shell.addFunction("env.baro.mmhg", getPressureMmhg);
  Shell.addFunction("env.baro.inhg", getPressureInhg);
  Shell.addFunction("env.light.ir", getLightIr);
  Shell.addFunction("env.light.visible", getLightVisible);
  Shell.addFunction("env.light.full", getLightFull);
  Shell.addFunction("env.light.lux", getLightLux);
  Shell.addFunction("env.light.config", lightConfig);
  Shell.addFunction("env.status", status);
  Serial.println("env enabled");

  /*
  for(int i=0; i<MOVAVG_SIZE; i++) {
    humidityPushMovingAvg(cc2->readHumidity());
    tempPushMovingAvg(cc2->readTemperature());
  }
  
  for(int i=0; i<MOVAVG_SIZE; i++) {
    baroPushMovingAvg(mpl->getPressure());
  }

  htuMovingAvgTimer.interval = 250;
  htuMovingAvgTimer.mode = SYS_TIMER_PERIODIC_MODE;
  htuMovingAvgTimer.handler = htuMovingAvgTimerHandler;
  SYS_TimerStart(&htuMovingAvgTimer);
    
  baroMovingAvgTimer.interval = 250;
  baroMovingAvgTimer.mode = SYS_TIMER_PERIODIC_MODE;
  baroMovingAvgTimer.handler = baroMovingAvgTimerHandler;
  SYS_TimerStart(&baroMovingAvgTimer);
  */
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
  return (1.8 * tempGetMovingAvgC()) + 32;
}

void EnvModule::tempPushMovingAvg(float val) {
  tempMovingAvgBuffer[tempMovingAvgCtr] = val;
  tempMovingAvgCtr = (tempMovingAvgCtr + 1) % MOVAVG_SIZE;
}

float EnvModule::baroGetMovingAvgKpa(void) {
  float sum = 0.0;
  for(int i=0; i<MOVAVG_SIZE; i++) {
    sum += baroMovingAvgBuffer[i];
  }
  return sum / MOVAVG_SIZE;
}

float EnvModule::baroGetMovingAvgMbar(void) {
  return baroGetMovingAvgKpa() * 10;
}

float EnvModule::baroGetMovingAvgMmhg(void) {
  return baroGetMovingAvgKpa() * 7.50062;
}

float EnvModule::baroGetMovingAvgInhg(void) {
  return baroGetMovingAvgKpa() * 0.2953;
}

void EnvModule::baroPushMovingAvg(float val) {
  baroMovingAvgBuffer[baroMovingAvgCtr] = val;
  baroMovingAvgCtr = (baroMovingAvgCtr + 1) % MOVAVG_SIZE;
}

uint16_t EnvModule::lightGetIr() {
  uint32_t lum = tsl->getFullLuminosity();
  uint16_t ir;
  ir = lum >> 16;
  return ir;
}

uint16_t EnvModule::lightGetVisible() {
  uint32_t lum = tsl->getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF; 
  return full-ir;
}

uint16_t EnvModule::lightGetFull() {
  uint32_t lum = tsl->getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF; 
  return full;
}

uint16_t EnvModule::lightGetLux() {
  uint32_t lum = tsl->getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF; 
  return tsl->calculateLux(full, ir);
}

bool EnvModule::lightConfigure(uint8_t sensitivity) {
  // bright light situations
  if (sensitivity == 0) {
    tsl->setGain(TSL2561_GAIN_0X);
    tsl->setTiming(TSL2561_INTEGRATIONTIME_13MS);
  } 
  // medium light situations
  else if (sensitivity == 1) {
    tsl->setGain(TSL2561_GAIN_0X);
    tsl->setTiming(TSL2561_INTEGRATIONTIME_101MS);
  }
  // dim light situations
  else if (sensitivity == 2) {
    tsl->setGain(TSL2561_GAIN_16X);
    tsl->setTiming(TSL2561_INTEGRATIONTIME_402MS);
  }
  
  return true;
}

