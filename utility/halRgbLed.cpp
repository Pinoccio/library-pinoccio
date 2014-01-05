/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD license as described in license.txt.         *
\**************************************************************************/

#include "halRgbLed.h"
#include "Arduino.h"
#include <avr/eeprom.h>
#include "utility/sysTimer.h"

HalRgbLed RgbLed;

HalRgbLed::HalRgbLed() {
  turnOff();
  enable();
  blinkState = false;

  redValue = greenValue = blueValue = 0;

  torchRedValue = eeprom_read_byte((uint8_t *)8127);
  torchGreenValue = eeprom_read_byte((uint8_t *)8128);
  torchBlueValue = eeprom_read_byte((uint8_t *)8129);

  blinkTimer.interval = 500;
  blinkTimer.mode = SYS_TIMER_INTERVAL_MODE;
  blinkTimer.handler = halRgbLedBlinkTimerHandler;
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

void HalRgbLed::enableContinuousBlink() {
  blinkTimer.mode = SYS_TIMER_PERIODIC_MODE;
}

void HalRgbLed::disableContinuousBlink() {
  blinkTimer.mode = SYS_TIMER_INTERVAL_MODE;
}

void HalRgbLed::turnOff() {
  setRedValue(0);
  setGreenValue(0);
  setBlueValue(0);
  blinkState = false;
  SYS_TimerStop(&blinkTimer);
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

void HalRgbLed::blinkRed(unsigned int ms) {
  blinkColor(255, 0, 0);
}

void HalRgbLed::blinkGreen(unsigned int ms) {
  blinkColor(0, 255, 0);
}

void HalRgbLed::blinkBlue(unsigned int ms) {
  blinkColor(0, 0, 255);
}

void HalRgbLed::blinkCyan(unsigned int ms) {
  blinkColor(0, 255, 255);
}

void HalRgbLed::blinkPurple(unsigned int ms) {
  blinkColor(50, 0, 255);
}

void HalRgbLed::blinkMagenta(unsigned int ms) {
  blinkColor(255, 0, 255);
}

void HalRgbLed::blinkYellow(unsigned int ms) {
  blinkColor(255, 255, 0);
}

void HalRgbLed::blinkOrange(unsigned int ms) {
  blinkColor(255, 127, 0);
}

void HalRgbLed::blinkWhite(unsigned int ms) {
  blinkColor(255, 255, 255);
}

void HalRgbLed::blinkTorchColor(unsigned int ms) {
  blinkColor(torchRedValue, torchGreenValue, torchBlueValue, ms);
}

void HalRgbLed::blinkColor(short red, short green, short blue, int ms) {
  if (!isEnabled()) {
    return;
  }
  blinkTimer.interval = ms;
  blinkState = true;
  setBlinkValues(red, green, blue);
  setRGBLED(red, green, blue);
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
  setRedValue(blinkRedValue);
  setGreenValue(blinkGreenValue);
  setBlueValue(blinkBlueValue);
}

void HalRgbLed::setColor(short red, short green, short blue) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRGBLED(red, green, blue);
}

void HalRgbLed::setRGBLED(short red, short green, short blue) {
  setRedValue(red);
  setGreenValue(green);
  setBlueValue(blue);
}

void HalRgbLed::setHex(char* hex) {
  uint8_t i, t, hn, ln, len;
  len = 6;
  uint8_t out[3];

  for (t=0,i=0; i<len; i+=2,++t) {
    hn = hex[i] > '9' ? (uint8_t)hex[i] - 'A' + 10 : (uint8_t)hex[i] - '0';
    ln = hex[i+1] > '9' ? (uint8_t)hex[i+1] - 'A' + 10 : (uint8_t)hex[i+1] - '0';
    out[t] = (uint8_t)(hn << 4 ) | (uint8_t)ln;
  }

  setRedValue(out[0]);
  setGreenValue(out[1]);
  setBlueValue(out[2]);
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
  setRedValue(torchRedValue);
  setGreenValue(torchGreenValue);
  setBlueValue(torchBlueValue);
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

static void halRgbLedBlinkTimerHandler(SYS_Timer_t *timer) {
  if (timer->mode == SYS_TIMER_PERIODIC_MODE) {
    if (RgbLed.blinkState == true) {
      RgbLed.blinkState = false;
      RgbLed.setRedValue(0);
      RgbLed.setGreenValue(0);
      RgbLed.setBlueValue(0);
    } else {
      RgbLed.blinkState = true;
      RgbLed.setLEDToBlinkValue();
    }
  } else {
    RgbLed.turnOff();
  }
}