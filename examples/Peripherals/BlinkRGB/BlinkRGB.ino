#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  RgbLed.blinkRed();
  delay(1000);
  RgbLed.blinkGreen();
  delay(1000);
  RgbLed.blinkBlue();
  delay(1000);
}