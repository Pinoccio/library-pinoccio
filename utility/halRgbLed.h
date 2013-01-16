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
    void magenta();
    void yellow();
    void white();

    void blinkRed(int ms=500);
    void blinkGreen(int ms=500);
    void blinkBlue(int ms=500);
    void blinkCyan(int ms=500);
    void blinkMagenta(int ms=500);
    void blinkYellow(int ms=500);
    void blinkWhite(int ms=500);

    void setRed(int value);
    void setGreen(int value);
    void setBlue(int value);

  protected:
    bool enabled;
};

extern HalRgbLed RgbLed;

#endif // _PINOCCIO_HAL_RGB_LED_H