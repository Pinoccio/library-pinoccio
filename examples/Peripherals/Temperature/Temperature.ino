#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  Serial.print("Temperature: ");
  Serial.println(Scout.getTemperature());

  delay(1000);
}