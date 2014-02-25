#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>

/* MANUAL PROVISIONING BITLASH COMMANDS
mesh.config(<Scout ID>, <Troop ID>)
mesh.key("<mesh encryption key with max 16 chars>")
scout.sethqtoken("<HQ Token String>")
wifi.config("<Access point name>", "<Access point password>")
wifi.dhcp // if dhcp
wifi.reassociate()
*/

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();
}
