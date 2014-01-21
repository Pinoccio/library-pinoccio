#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  Serial.println("blink red once for 100ms");
  RgbLed.blinkRed(100);
  Scout.delay(1500);

  Serial.println("blink green once using the default (500ms)");
  RgbLed.blinkGreen();
  Scout.delay(1500);
  
  Serial.println("blink blue once for 1000ms");
  RgbLed.blinkBlue(1000);
  Scout.delay(1500);

  Serial.println("blink yellow continuously every 100ms");
  RgbLed.enableContinuousBlink();
  RgbLed.blinkYellow(100);
  Scout.delay(1500);
  RgbLed.disableContinuousBlink();
  Scout.delay(1000);
}