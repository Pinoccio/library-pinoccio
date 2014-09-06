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
#include "FastLED/FastLED.h"
#include "Pixels.h"

using namespace pinoccio;

PixelsModule PixelsModule::instance;

const __FlashStringHelper *PixelsModule::name() const {
  return F("pixels");
}

// Define the array of leds
CRGB *pixels = NULL;
uint8_t num_leds = 0;

// Exposing to Bitlash
static numvar pixelsSetRGB(void) {
  
  FastLED.clear();
  for (int i=0; i<num_leds; i++) {
    pixels[i].setRGB(getarg(1), getarg(2), getarg(3));
  }
  FastLED.show();
  return 1;
}
static numvar pixelsSetHSV(void) {
  FastLED.clear();
  for (int i=0; i<num_leds; i++) {
    pixels[i].setHSV(getarg(1), getarg(2), getarg(3));
  }
  FastLED.show();
  return 1;
}
static numvar pixelsSetHue(void) {
  FastLED.clear();
  for (int i=0; i<num_leds; i++) {
    pixels[i].setHue(getarg(1));
  }
  FastLED.show();
  return 1;
}
static numvar pixelsSetBrightness(void) {
  FastLED.clear();
  for (int i=0; i<num_leds; i++) {
    pixels[i].fadeToBlackBy(getarg(1)*255/100);
  }
  FastLED.show();
  return 1;
}
static numvar pixelsOff(void) {
  FastLED.clear();
  FastLED.show();
  return 1;
}
static numvar pixelsConfig(void) {
  if (!checkArgs(1, 4, F("usage: pixels.add(pixels [,2,\"neopixel\",\"rgb\"])"))) {
    return 0;
  }
  num_leds = getarg(1);

  uint8_t pin = 2;
  if(getarg(0) > 1)
  {
    pin = getarg(2);
  }

  EClocklessChipsets chip = NEOPIXEL;
  if(getarg(0) > 2)
  {
    char *arg = (char*)getstringarg(3);
    // TODO add other string mappings, hopefully there's a better way?
    if(strcasecmp(arg,"neopixel") == 0) chip = NEOPIXEL;
  }

  EOrder order = RGB;
  if(getarg(0) > 3)
  {
    char *arg = (char*)getstringarg(4);
    // TODO add other string mappings, hopefully there's a better way?
    if(strcasecmp(arg,"rgb") == 0) order = RGB;
  }

  free(pixels);
  pixels = (CRGB*)malloc(sizeof (CRGB)*num_leds);

  // I don't know c++/templates so this is a non-scaleable workaround
  if(chip == NEOPIXEL && pin == 2 && order == RGB) FastLED.addLeds<NEOPIXEL, 2, RGB>(pixels, num_leds);

  FastLED.clear();
  return 1;
}
  
bool PixelsModule::enable() {

  Shell.addFunction("pixels.add", pixelsConfig);
  Shell.addFunction("pixels.setrgb", pixelsSetRGB);
  Shell.addFunction("pixels.sethsv", pixelsSetHSV);
  Shell.addFunction("pixels.sethue", pixelsSetHue);
  Shell.addFunction("pixels.setbrightness", pixelsSetBrightness);
  Shell.addFunction("pixels.off", pixelsOff);
}

void PixelsModule::loop() { }

