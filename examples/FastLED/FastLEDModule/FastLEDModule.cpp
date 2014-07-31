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
#include "FastLEDModule.h"

// How many leds in your strip?
#define NUM_LEDS 8

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 2

// Define the array of leds
CRGB pixels[NUM_LEDS];

// Exposing to Bitlash
static numvar pixelsSetRGB(void) {
  FastLED.clear();
  for (int i=0; i<NUM_LEDS; i++) {
    pixels[i].setRGB(getarg(1), getarg(2), getarg(3));
  }
  FastLED.show();
  return 1;
}
static numvar pixelsSetHSV(void) {
  FastLED.clear();
  for (int i=0; i<NUM_LEDS; i++) {
    pixels[i].setHSV(getarg(1), getarg(2), getarg(3));
  }
  FastLED.show();
  return 1;
}
static numvar pixelsSetHue(void) {
  FastLED.clear();
  for (int i=0; i<NUM_LEDS; i++) {
    pixels[i].setHue(getarg(1));
  }
  FastLED.show();
  return 1;
}
void FastLEDModule::setHue(int hue) {
  FastLED.clear();
  for (int i=0; i<NUM_LEDS; i++) {
    pixels[i].setHue(hue);
  }
  FastLED.show();
}

static numvar pixelsSetBrightness(void) {
  FastLED.clear();
  for (int i=0; i<NUM_LEDS; i++) {
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

void FastLEDModule::setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN, GRB>(pixels, NUM_LEDS);

  addBitlashFunction("pixels.setrgb", (bitlash_function) pixelsSetRGB);
  addBitlashFunction("pixels.sethsv", (bitlash_function) pixelsSetHSV);
  addBitlashFunction("pixels.sethue", (bitlash_function) pixelsSetHue);
  addBitlashFunction("pixels.setbrightness", (bitlash_function) pixelsSetBrightness);
  addBitlashFunction("pixels.off", (bitlash_function) pixelsOff);
}

void FastLEDModule::loop() { }
