#include <Arduino.h>
#include <Backpack.h>

PinoccioBackpack::PinoccioBackpack() : family(0), id(0) { }

PinoccioBackpack::~PinoccioBackpack() { }

void PinoccioBackpack::setup() { }

void PinoccioBackpack::loop() { }

void PinoccioBackpack::dumpBackpacks() {
  Serial.print("Found ");
  Serial.print(Scout.bp.num_slaves);
  Serial.println(" slaves");

  for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
    print_hex(Scout.bp.slave_ids[i], sizeof(Scout.bp.slave_ids[0]));
    Serial.println();
    uint8_t buf[64];
    Scout.bp.readEeprom(i + 1, 0, buf, sizeof(buf));
    Serial.print("EEPROM: ");
    print_hex(buf, sizeof(buf));
    Serial.println();
  }
}