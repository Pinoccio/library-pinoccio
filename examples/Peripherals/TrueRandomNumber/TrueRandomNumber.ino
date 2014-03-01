#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();

  Serial.print("Random Number: ");
  Serial.println(random());

  Scout.delay(1000);
}
