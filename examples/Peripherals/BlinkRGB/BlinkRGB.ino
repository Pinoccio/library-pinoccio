#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  RgbLed.blinkRed();
  Scout.delay(1000);
  RgbLed.blinkGreen();
  Scout.delay(1000);
  RgbLed.blinkBlue();
  Scout.delay(1000);
}