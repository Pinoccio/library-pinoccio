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

HalRgbLed RgbLed;

HalRgbLed::HalRgbLed() {
  turnOff();
  enable();
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
  setRed(0);
  setGreen(0);
  setBlue(0);
}

void HalRgbLed::red() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRed(255);
}

void HalRgbLed::green() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setGreen(255);
}

void HalRgbLed::blue() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setBlue(255);
}

void HalRgbLed::cyan() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setGreen(255);
  setBlue(255);
}

void HalRgbLed::magenta() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRed(255);
  setBlue(255);
}

void HalRgbLed::yellow() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRed(255);
  setGreen(255);
}

void HalRgbLed::white() {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  setRed(255);
  setGreen(255);
  setBlue(255);
}

void HalRgbLed::blinkRed(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  red();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkGreen(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  green();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkBlue(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  blue();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkCyan(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  cyan();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkMagenta(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  magenta();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkYellow(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  yellow();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::blinkWhite(int ms) {
  if (!isEnabled()) {
    return;
  }
  turnOff();
  white();
  delay(ms);
  turnOff();
  delay(ms);
}

void HalRgbLed::setRed(int value) {
  if (isEnabled()) {
    analogWrite(LED_RED, 255-value);
  }
}

void HalRgbLed::setGreen(int value) {
  if (isEnabled()) {
    analogWrite(LED_GREEN, 255-value);
  }
}

void HalRgbLed::setBlue(int value) {
  if (isEnabled()) {
    analogWrite(LED_BLUE, 255-value);
  }
}

void HalRgbLed::setHex(char* hex) {
  unsigned int i, t, hn, ln;
  int len = 6;
  uint8_t out[3];
  
  for (t=0,i=0; i<len; i+=2,++t) {  
    hn = hex[i] > '9' ? hex[i] - 'A' + 10 : hex[i] - '0';
    ln = hex[i+1] > '9' ? hex[i+1] - 'A' + 10 : hex[i+1] - '0';  
    out[t] = (hn << 4 ) | ln;
  }

  setRed(out[0]);
  setGreen(out[1]);
  setBlue(out[2]);
}