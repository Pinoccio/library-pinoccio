#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  Serial.print("Random Number: ");
  Serial.println(Scout.getRandomNumber());

  Scout.delay(1000);
}