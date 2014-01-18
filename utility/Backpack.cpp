#include <Arduino.h>
#include <Scout.h>
#include <Backpack.h>

PinoccioBackpack::PinoccioBackpack() : version(0), model(0), revision(0), id(0) { }

PinoccioBackpack::~PinoccioBackpack() { }

void PinoccioBackpack::setup() { }

void PinoccioBackpack::loop() { }

void PinoccioBackpack::dumpBackpacks() {
  Serial.print("Found ");
  Serial.print(Scout.bp.num_slaves);
  Serial.println(" slaves");

  for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
    printHex(Scout.bp.slave_ids[i], sizeof(Scout.bp.slave_ids[0]));
    Serial.println();
    uint8_t buf[64];
    Scout.bp.readEeprom(i + 1, 0, buf, sizeof(buf));
    Serial.print("EEPROM: ");
    printHex(buf, sizeof(buf));
    Serial.println();
  }
}

void PinoccioBackpack::printHex(const uint8_t *buf, uint8_t len) {
  while (len--) {
    if (*buf < 0x10) Serial.print("0");
    Serial.print(*buf++, HEX);
  }
}