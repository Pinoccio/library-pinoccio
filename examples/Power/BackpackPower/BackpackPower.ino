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

  Scout.enableBackpackVcc();
  Serial.println("Backpack VCC is on.");
  Scout.delay(3000);

  Scout.disableBackpackVcc();
  Serial.println("Backpack VCC is off.");
  Scout.delay(3000);
}
