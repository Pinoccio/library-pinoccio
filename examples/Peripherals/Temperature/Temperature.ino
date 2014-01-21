#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  int tempInCelsius = Scout.getTemperature();
  int tempInFahrenheit = tempInCelsius * 9/5 + 32;
  Serial.print("Temperature: ");
  Serial.print(tempInCelsius);
  Serial.print(" C, ");
  Serial.print(tempInFahrenheit);
  Serial.println(" F");

  Scout.delay(1000);
}