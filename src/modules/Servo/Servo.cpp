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
#include "Servo.h"
#include "TimerThree.h"

using namespace pinoccio;

ServoModule ServoModule::instance;

const __FlashStringHelper *ServoModule::name() const {
  return F("servo");
}

// this was from TimerThree.cpp and changed to be dynamic not static
TimerThree *Timer3;

unsigned short TimerThree::pwmPeriod = 0;
unsigned char TimerThree::clockSelectBits = 0;
void (*TimerThree::isrCallback)() = NULL;

// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER3_OVF_vect)
{
  Timer3->isrCallback();
}

static numvar servoInitialize() {
  Timer3->initialize(getarg(1));
}

static numvar servoSetPeriod() {
  Timer3->setPeriod(getarg(1));
}

static numvar servoStart() {
  Timer3->start();
}

static numvar servoStop() {
  Timer3->stop();
}

static numvar servoRestart() {
  Timer3->restart();
}

static numvar servoResume() {
  Timer3->resume();
}

static numvar servoPwm() {
  Timer3->pwm(getarg(1), getarg(2));
}

static numvar servoSetPwmDuty() {
  Timer3->setPwmDuty(getarg(1), getarg(2));
}

static numvar servoDisablePwm() {
  Timer3->disablePwm(getarg(1));
}

bool ServoModule::enable() {
  Timer3 = new TimerThree;
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

