#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>

/* MANUAL PROVISIONING BITLASH COMMANDS
mesh.config(<Scout ID>, <Troop ID>)
mesh.key("<mesh encryption key with max 16 chars>")
wifi.config("<Access point name>", "<Access point password>", "173.255.220.185", 22756)
scout.sethqtoken("<HQ Token String>")
*/

void setup() {
  // pass setup(1) to force lead scout
  Scout.setup(1);
}

void loop() {
  Scout.loop();
}