#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>

void setup() {
  Scout.setup();
}

int red, green, blue;
long time = 0;

void loop() {
  Scout.loop();

  time = millis();
  red = 128 + 127 * cos(2 * PI / 2000 * time);
  green = 128 + 127 * cos(2 * PI / 2000 * (500 - time));
  blue = 128 + 127 * cos(2 * PI / 2000 * (1000 - time));

  RgbLed.setRedValue(red);
  RgbLed.setGreenValue(green);
  RgbLed.setBlueValue(blue);
}