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

  delay(1000);
}