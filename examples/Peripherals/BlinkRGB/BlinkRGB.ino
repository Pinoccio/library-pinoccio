#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  // blink once for 250ms
  RgbLed.blinkRed(250);
  Scout.delay(1000);

  // blink once using the default (500ms)
  RgbLed.blinkGreen();
  Scout.delay(1000);
  
  // blink once using 750ms
  RgbLed.blinkBlue(750);
  Scout.delay(1000);
}