#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

/* MANUAL PROVISIONING BITLASH COMMANDS
mesh.config(<Scout ID>, <Troop ID>)
wifi.config("<Access point name>", "<Access point password>", "<API IP Address>", <API Port>)
scout.sethqtoken("<HQ Token String>")
*/

void setup() {
  // pass setup(1) to force lead scout
  Scout.setup();
}

void loop() {
  Scout.loop();
}