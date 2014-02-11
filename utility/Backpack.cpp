#include <Arduino.h>
#include <Scout.h>
#include <Backpack.h>

PinoccioBackpack::PinoccioBackpack() : version(0), model(0), revision(0), id(0) { }

PinoccioBackpack::~PinoccioBackpack() { }

void PinoccioBackpack::setup() { }

void PinoccioBackpack::loop() { }

void PinoccioBackpack::printHex(const uint8_t *buf, uint8_t len) {
  while (len--) {
    if (*buf < 0x10) Serial.print("0");
    Serial.print(*buf++, HEX);
  }
}
