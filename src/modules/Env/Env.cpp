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
  speol(m->tempGetC(), 1);
  sp("Temp F  : ");
  speol(m->tempGetF(), 1);
  sp("Humidity: ");
  speol(m->humidityGetPercent(), 1);
  sp("Pressure kPa: ");
  speol(m->baroGetKpa(), 1);
  sp("Pressure mbar: ");
  speol(m->baroGetMbar(), 1);
  sp("Pressure mmHg: ");
  speol(m->baroGetMmHg(), 1);
  sp("Pressure mmIn: ");
  speol(m->baroGetInHg(), 1);
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
  return (int)round(m->tempGetC());
}

static numvar getTemperatureF() {
  EnvModule* m = &(EnvModule::instance);
  return  (int)round(m->tempGetF());
}

static numvar getHumidity() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->humidityGetPercent());
}

static numvar getPressureKpa() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->baroGetKpa());
}

static numvar getPressureMbar() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->baroGetMbar() * 10);
}

static numvar getPressureMmhg() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->baroGetMmHg() * 7.50062);
}

static numvar getPressureInhg() {
  EnvModule* m = &(EnvModule::instance);
  return (int)round(m->baroGetInHg() * 0.2953);
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

bool EnvModule::enable() {
  htu = new Adafruit_HTU21DF();
  htu->begin();
  
  mpl = new Adafruit_MPL115A2();
  mpl->begin();
  
  tsl = new TSL2561(TSL2561_ADDR_FLOAT);
  tsl->begin();

  Shell.addFunction("env.temp.c", getTemperatureC);
  Shell.addFunction("env.temp.f", getTemperatureF);
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
}

void EnvModule::loop() { }

float EnvModule::tempGetC(void) {
  return htu->readTemperature();
}

float EnvModule::tempGetF(void) {
  return (1.8 * htu->readTemperature()) + 32.0;
}

float EnvModule::humidityGetPercent(void) {
  return htu->readHumidity();
}

float EnvModule::baroGetKpa(void) {
  return mpl->getPressure();
}

float EnvModule::baroGetMbar(void) {
  return baroGetKpa() * 10;
}

float EnvModule::baroGetMmHg(void) {
  return baroGetKpa() * 7.50062;
}

float EnvModule::baroGetInHg(void) {
  return baroGetKpa() * 0.2953;
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

