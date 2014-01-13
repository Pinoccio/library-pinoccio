#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

void setup() {
  Scout.setup();
  RgbLed.blinkTorch();
}

void loop() {
  Scout.loop();
}