#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  int red, green, blue;
  long time = 0;

  time = millis();
  red = 128 + 127 * cos(2 * PI / 2000 * time);
  green = 128 + 127 * cos(2 * PI / 2000 * (500 - time));
  blue = 128 + 127 * cos(2 * PI / 2000 * (1000 - time));

  RgbLed.setRedValue(red);
  RgbLed.setGreenValue(green);
  RgbLed.setBlueValue(blue);
}