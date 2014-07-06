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
  addBitlashFunction("servo.initialize", (bitlash_function)servoInitialize);
  addBitlashFunction("servo.setPeriod", (bitlash_function)servoSetPeriod);
  addBitlashFunction("servo.start", (bitlash_function)servoStart);
  addBitlashFunction("servo.stop", (bitlash_function)servoStop);
  addBitlashFunction("servo.restart", (bitlash_function)servoRestart);
  addBitlashFunction("servo.resume", (bitlash_function)servoResume);
  addBitlashFunction("servo.pwm", (bitlash_function)servoPwm);
  addBitlashFunction("servo.setPwmDuty", (bitlash_function)servoSetPwmDuty);
  addBitlashFunction("servo.disablePwm", (bitlash_function)servoDisablePwm);
}

void ServoModule::loop() { }

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