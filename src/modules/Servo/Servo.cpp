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
#include "modules/Servo/Servo.h"
#include "TimerThree.h"

// TODO Timer3 should be created dynamically when the module is loaded, not static

static numvar servoInitialize();
static numvar servoSetPeriod();
static numvar servoStart();
static numvar servoStop();
static numvar servoRestart();
static numvar servoResume();
static numvar servoPwm();
static numvar servoSetPwmDuty();
static numvar servoDisablePwm();

ServoModule::ServoModule() { }

ServoModule::~ServoModule() { }

void ServoModule::setup() {
  Shell.addFunction("servo.initialize", servoInitialize);
  Shell.addFunction("servo.setPeriod", servoSetPeriod);
  Shell.addFunction("servo.start", servoStart);
  Shell.addFunction("servo.stop", servoStop);
  Shell.addFunction("servo.restart", servoRestart);
  Shell.addFunction("servo.resume", servoResume);
  Shell.addFunction("servo.pwm", servoPwm);
  Shell.addFunction("servo.setPwmDuty", servoSetPwmDuty);
  Shell.addFunction("servo.disablePwm", servoDisablePwm);
}

void ServoModule::loop() { }

const char *ServoModule::name() {
  return "servo";
}

static numvar servoInitialize() {
  Timer3.initialize(getarg(1));
}

static numvar servoSetPeriod() {
  Timer3.setPeriod(getarg(1));
}

static numvar servoStart() {
  Timer3.start();
}

static numvar servoStop() {
  Timer3.stop();
}

static numvar servoRestart() {
  Timer3.restart();
}

static numvar servoResume() {
  Timer3.resume();
}

static numvar servoPwm() {
  Timer3.pwm(getarg(1), getarg(2));
}

static numvar servoSetPwmDuty() {
  Timer3.setPwmDuty(getarg(1), getarg(2));
}

static numvar servoDisablePwm() {
  Timer3.disablePwm(getarg(1));
}