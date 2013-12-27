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
static SYS_Timer_t blinkTimer;

HalRgbLed::HalRgbLed() {
  turnOff();
  enable();
  redValue = greenValue = blueValue = 0;
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

void HalRgbLed::turnOff() {
  setRedValue(0);
  setGreenValue(0);
  setBlueValue(0);
}

void HalRgbLed::red() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRedValue(255);
}

void HalRgbLed::green() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setGreenValue(255);
}

void HalRgbLed::blue() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setBlueValue(255);
}

void HalRgbLed::cyan() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setGreenValue(255);
  setBlueValue(255);
}

void HalRgbLed::purple() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRedValue(50);
  setBlueValue(255);
}

void HalRgbLed::magenta() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRedValue(255);
  setBlueValue(255);
}

void HalRgbLed::yellow() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRedValue(255);
  setGreenValue(255);
}

void HalRgbLed::orange() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRedValue(255);
  setGreenValue(127);
}

void HalRgbLed::white() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRedValue(255);
  setGreenValue(255);
  setBlueValue(255);
}

void HalRgbLed::blinkRed(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  blinkTimer.interval = ms;
  setRedValue(255);
  SYS_TimerStart(&blinkTimer);
}

void HalRgbLed::blinkGreen(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  green();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkBlue(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  blue();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkCyan(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  cyan();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkPurple(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  purple();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkMagenta(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  magenta();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkYellow(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  yellow();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkOrange(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  orange();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkWhite(unsigned int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  white();
  delay(ms);
  turnOff();
  delay(ms);
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
}

void HalRgbLed::setTorch(void) {
  setRedValue(eeprom_read_byte((uint8_t *)8127));
  setGreenValue(eeprom_read_byte((uint8_t *)8128));
  setBlueValue(eeprom_read_byte((uint8_t *)8129));
}

short HalRgbLed::getRedTorchValue(void) {
  return eeprom_read_byte((uint8_t *)8127);
}

short HalRgbLed::getGreenTorchValue(void) {
  return eeprom_read_byte((uint8_t *)8128);
}

short HalRgbLed::getBlueTorchValue(void) {
  return eeprom_read_byte((uint8_t *)8129);
}

static void halRgbLedBlinkTimerHandler(SYS_Timer_t *timer) {
  turnOff();
}