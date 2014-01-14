#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

/* MANUAL PROVISIONING BITLASH COMMANDS
mesh.config()
wifi.config()
scout.sethqtoken()
*/

void setup() {
  // pass setup(1) to force lead scout
  Scout.setup();
}

void loop() {
  Scout.loop();
}