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

    void blinkRed(unsigned int ms=500);
    void blinkGreen(unsigned int ms=500);
    void blinkBlue(unsigned int ms=500);
    void blinkCyan(unsigned int ms=500);
    void blinkPurple(unsigned int ms=500);
    void blinkMagenta(unsigned int ms=500);
    void blinkYellow(unsigned int ms=500);
    void blinkOrange(unsigned int ms=500);
    void blinkWhite(unsigned int ms=500);

    void setRedValue(int value);
    void setGreenValue(int value);
    void setBlueValue(int value);

    int getRedValue();
    int getGreenValue();
    int getBlueValue();

    void setHex(char* hex);
    void saveTorch(short red, short green, short blue);
    void setTorch(void);

    short getRedTorchValue(void);
    short getGreenTorchValue(void);
    short getBlueTorchValue(void);

  protected:
    bool enabled;
    int redValue;
    int greenValue;
    int blueValue;

    int blinkRedValue;
    int blinkGreenValue;
    int blinkBlueValue;
};

extern HalRgbLed RgbLed;
extern SYS_Timer_t blinkTimer;
static void halRgbLedBlinkTimerHandler(SYS_Timer_t *timer);

#endif // _PINOCCIO_HAL_RGB_LED_H