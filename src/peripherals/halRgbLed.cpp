/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include "halRgbLed.h"
#include "Arduino.h"
#include "Scout.h"
#include <avr/eeprom.h>
#include "lwm/sys/sysTimer.h"

static void halRgbLedBlinkTimerHandler(SYS_Timer_t *timer);

HalRgbLed Led;

HalRgbLed::HalRgbLed() {
  turnOff();
  enable();

  redValue = greenValue = blueValue = 0;

  torchRedValue = eeprom_read_byte((uint8_t *)8127);
  torchGreenValue = eeprom_read_byte((uint8_t *)8128);
  torchBlueValue = eeprom_read_byte((uint8_t *)8129);

  blinkTimer.interval = 500;
  blinkTimer.handler = halRgbLedBlinkTimerHandler;

  ledEventHandler = 0;
}

void HalRgbLed::enable() {
  enabled = true;
}

void HalRgbLed::disable() {
  turnOff();
  enabled = false;
}

bool HalRgbLed::isEnabled() {
  return enabled;
}

void HalRgbLed::turnOff() {
  setColor(0, 0, 0);
  blinkTimer.mode = SYS_TIMER_INTERVAL_MODE;
  SYS_TimerStop(&blinkTimer);
}

bool HalRgbLed::isOff() {
  return (getRedValue() == 0 && getGreenValue() == 0 && getBlueValue() == 0);
}

void HalRgbLed::red() {
  setColor(255, 0, 0);
}

void HalRgbLed::green() {
  setColor(0, 255, 0);
}

void HalRgbLed::blue() {
  setColor(0, 0, 255);
}

void HalRgbLed::cyan() {
  setColor(0, 255, 255);
}

void HalRgbLed::purple() {
  setColor(50, 0, 255);
}

void HalRgbLed::magenta() {
  setColor(255, 0, 255);
}

void HalRgbLed::yellow() {
  setColor(255, 255, 0);
}

void HalRgbLed::orange() {
  setColor(255, 127, 0);
}

void HalRgbLed::white() {
  setColor(255, 255, 255);
}

void HalRgbLed::blinkRed(unsigned int ms, bool continuous) {
  blinkColor(255, 0, 0, ms, continuous);
}

void HalRgbLed::blinkGreen(unsigned int ms, bool continuous) {
  blinkColor(0, 255, 0, ms, continuous);
}

void HalRgbLed::blinkBlue(unsigned int ms, bool continuous) {
  blinkColor(0, 0, 255, ms, continuous);
}

void HalRgbLed::blinkCyan(unsigned int ms, bool continuous) {
  blinkColor(0, 255, 255, ms, continuous);
}

void HalRgbLed::blinkPurple(unsigned int ms, bool continuous) {
  blinkColor(50, 0, 255, ms, continuous);
}

void HalRgbLed::blinkMagenta(unsigned int ms, bool continuous) {
  blinkColor(255, 0, 255, ms, continuous);
}

void HalRgbLed::blinkYellow(unsigned int ms, bool continuous) {
  blinkColor(255, 255, 0, ms, continuous);
}

void HalRgbLed::blinkOrange(unsigned int ms, bool continuous) {
  blinkColor(255, 127, 0, ms, continuous);
}

void HalRgbLed::blinkWhite(unsigned int ms, bool continuous) {
  blinkColor(255, 255, 255, ms, continuous);
}

void HalRgbLed::blinkTorch(unsigned int ms, bool continuous) {
  blinkColor(torchRedValue, torchGreenValue, torchBlueValue, ms, continuous);
}

void HalRgbLed::blinkColor(short red, short green, short blue, unsigned int ms, bool continuous) {
  if (!isEnabled()) {
    return;
  }
  if (continuous) {
    blinkTimer.mode = SYS_TIMER_PERIODIC_MODE;
  } else {
    blinkTimer.mode = SYS_TIMER_INTERVAL_MODE;
  }
  blinkTimer.interval = ms;
  setBlinkValues(red, green, blue);
  setColor(red, green, blue);
  SYS_TimerStart(&blinkTimer);
}

void HalRgbLed::setRedValue(int value) {
  if (isEnabled()) {
    analogWrite(LED_RED, 255-value);
    redValue = value;
  }
}

void HalRgbLed::setGreenValue(int value) {
  if (isEnabled()) {
    analogWrite(LED_GREEN, 255-value);
    greenValue = value;
  }
}

void HalRgbLed::setBlueValue(int value) {
  if (isEnabled()) {
    analogWrite(LED_BLUE, 255-value);
    blueValue = value;
  }
}

int HalRgbLed::getRedValue() {
  return redValue;
}

int HalRgbLed::getGreenValue() {
  return greenValue;
}

int HalRgbLed::getBlueValue() {
  return blueValue;
}

void HalRgbLed::setBlinkValues(short red, short green, short blue) {
  blinkRedValue = red;
  blinkGreenValue = green;
  blinkBlueValue = blue;
}

void HalRgbLed::setLEDToBlinkValue() {
  setColor(blinkRedValue, blinkGreenValue, blinkBlueValue);
}

void HalRgbLed::setColor(short red, short green, short blue) {
  if (!isEnabled()) {
    return;
  }
  bool hasChanged = false;
  if (red != getRedValue() || green != getGreenValue() || blue != getBlueValue()) {
    hasChanged = true;
  }

  setRedValue(red);
  setGreenValue(green);
  setBlueValue(blue);

  if (hasChanged) {
    triggerEvent();
  }
}

void HalRgbLed::saveTorch(short red, short green, short blue) {
  eeprom_update_byte((uint8_t *)8127, red);
  eeprom_update_byte((uint8_t *)8128, green);
  eeprom_update_byte((uint8_t *)8129, blue);

  torchRedValue = red;
  torchGreenValue = green;
  torchBlueValue = blue;
}

void HalRgbLed::setTorch(void) {
  setColor(torchRedValue, torchGreenValue, torchBlueValue);
}

short HalRgbLed::getRedTorchValue(void) {
  return torchRedValue;
}

short HalRgbLed::getGreenTorchValue(void) {
  return torchGreenValue;
}

short HalRgbLed::getBlueTorchValue(void) {
  return torchBlueValue;
}

void HalRgbLed::triggerEvent(void) {
  if (Led.ledEventHandler != 0) {
    if (Scout.eventVerboseOutput) {
      Serial.print(F("Running: ledEventHandler("));
      Serial.print(redValue);
      Serial.print(F(","));
      Serial.print(greenValue);
      Serial.print(F(","));
      Serial.print(blueValue);
      Serial.println(F(")"));
    }
    Led.ledEventHandler(redValue, greenValue, blueValue);
  }
}

static void halRgbLedBlinkTimerHandler(SYS_Timer_t *timer) {
  if (timer->mode == SYS_TIMER_PERIODIC_MODE) {
    if (Led.getRedValue() || Led.getGreenValue() || Led.getBlueValue()) {
      Led.setColor(0, 0, 0);
    } else {
      Led.setLEDToBlinkValue();
    }

  } else {
    Led.turnOff();
  }
}
