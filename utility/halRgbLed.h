/**************************************************************************\
* Pinoccio Arduino Library                                                 *
* https://github.com/Pinoccio/pinoccio-arduino-library                     *
* Copyright (c) 2012-2013, Pinoccio. All rights reserved.                  *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD license as described in license.txt.         *
\**************************************************************************/

#ifndef _PINOCCIO_HAL_RGB_LED_H
#define _PINOCCIO_HAL_RGB_LED_H

#include "utility/sysTimer.h"

class HalRgbLed {
  public:
    HalRgbLed();

    void enable();
    void disable();
    bool isEnabled();
    void turnOff();

    void red();
    void green();
    void blue();
    void cyan();
    void purple();
    void magenta();
    void yellow();
    void orange();
    void white();

    void blinkRed(unsigned int ms=500, bool continuous=false);
    void blinkGreen(unsigned int ms=500, bool continuous=false);
    void blinkBlue(unsigned int ms=500, bool continuous=false);
    void blinkCyan(unsigned int ms=500, bool continuous=false);
    void blinkPurple(unsigned int ms=500, bool continuous=false);
    void blinkMagenta(unsigned int ms=500, bool continuous=false);
    void blinkYellow(unsigned int ms=500, bool continuous=false);
    void blinkOrange(unsigned int ms=500, bool continuous=false);
    void blinkWhite(unsigned int ms=500, bool continuous=false);
    void blinkTorch(unsigned int ms=500, bool continuous=false);
    void blinkColor(short red, short green, short blue, unsigned int ms=500, bool continuous=false);

    int getRedValue();
    int getGreenValue();
    int getBlueValue();

    void setBlinkValues(short red, short green, short blue);
    void setLEDToBlinkValue();
    void setColor(short red, short green, short blue);
    void saveTorch(short red, short green, short blue);
    void setTorch(void);

    short getRedTorchValue(void);
    short getGreenTorchValue(void);
    short getBlueTorchValue(void);

    void triggerEvent();
    void (*ledEventHandler)(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);

  protected:
    void setRedValue(int value);
    void setGreenValue(int value);
    void setBlueValue(int value);

    bool enabled;

    short redValue;
    short greenValue;
    short blueValue;

    short blinkRedValue;
    short blinkGreenValue;
    short blinkBlueValue;

    short torchRedValue;
    short torchGreenValue;
    short torchBlueValue;

    SYS_Timer_t blinkTimer;
};

extern HalRgbLed RgbLed;
static void halRgbLedBlinkTimerHandler(SYS_Timer_t *timer);

#endif // _PINOCCIO_HAL_RGB_LED_H
