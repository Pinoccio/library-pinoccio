#include <Pinoccio.h>

void setup() { }

void loop() {
  int red, green, blue;
  long time = 0;

  time = millis();
  red = 128 + 127 * cos(2 * PI / 2000 * time);
  green = 128 + 127 * cos(2 * PI / 2000 * (500 - time));
  blue = 128 + 127 * cos(2 * PI / 2000 * (1000 - time));

  RgbLed.setRed(red);
  RgbLed.setGreen(green);
  RgbLed.setBlue(blue);
}