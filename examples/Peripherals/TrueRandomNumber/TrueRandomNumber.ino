#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  Serial.print("Random Number: ");
  Serial.println(random());

  Scout.delay(1000);
}