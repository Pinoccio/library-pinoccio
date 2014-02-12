#include <Arduino.h>
#include "Backpacks.h"

uint8_t Backpacks::num_backpacks = 0;
BackpackInfo *Backpacks::info = NULL;
PBBP Backpacks::pbbp;

void Backpacks::setup()
{
  // Give the slaves on the backpack bus a bit of time to start up. 1ms
  // seems to be enough, but let's be generous.
  delay(5);
  pbbp.begin(BACKPACK_BUS);
  detect();
}

void Backpacks::detect()
{
  free(info);
  num_backpacks = 0;
  if (!pbbp.enumerate(addBackpack))
    printPbbpError("Backpack enumeration failed: ");
}

void Backpacks::printPbbpError(const char *prefix)
{
  Serial.print(prefix);
  pbbp.printLastError(Serial);
  Serial.println();
}

uint8_t *BackpackInfo::getEeprom()
{
  if (this->eeprom_contents)
    return this->eeprom_contents;

  uint8_t buf[3];
  uint8_t addr = getAddress();
  // Read the first 3 bytes
  if (!Backpacks::pbbp.readEeprom(addr, 0, buf, sizeof(buf))) {
    Backpacks::printPbbpError("EEPROM read failed: ");
    return NULL;
  }
  // Check EEPROM version
  if (buf[0] > 1) {
    Serial.print("Unsupported EEPROM version: ");
    Serial.print(buf[0]);
    return NULL;
  }

  // Get the used size of the EEPROM
  uint8_t used_size = buf[2];
  // Allocate memory for that
  uint8_t *data = (uint8_t*)malloc(used_size);
  if (!data) {
    Serial.println("Memory allocation for EEPROM failed");
    Serial.println(used_size);
    return NULL;
  }

  // And read the full EEPROM
  if (!Backpacks::pbbp.readEeprom(addr, 0, data, used_size)) {
    Backpacks::printPbbpError("EEPROM read failed: ");
    free(data);
    return NULL;
  }

  this->eeprom_contents = data;
  this->eeprom_contents_length = used_size;
  return data;
}

void BackpackInfo::freeEeprom()
{
  free(this->eeprom_contents);
  this->eeprom_contents = NULL;
  this->eeprom_contents_length = 0;
}

uint8_t BackpackInfo::getAddress()
{
  // Deduce our address based on our place in the Backpacks::info array.
  return this - Backpacks::info;
}

void Backpacks::addBackpack(uint8_t *unique_id)
{
  info = (BackpackInfo*)realloc(info, (num_backpacks + 1) * sizeof(*info));
  BackpackInfo &bp = info[num_backpacks++];
  // TODO: This stuff might better belong in a constructor of
  // BackpackInfo (which is not current called!). However, doing this
  // nicely requires the placement new operator, which Arduino does not
  // (yet) supply. https://github.com/arduino/Arduino/pull/108
  bp.eeprom_contents = NULL;
  bp.eeprom_contents_length = 0;

  memcpy(bp.id.raw_bytes, unique_id, sizeof(bp.id));
}

bool Backpacks::isModelPresent(uint16_t modelid)
{
  for (uint8_t i = 0; i < num_backpacks; ++i) {
    if (modelid == info[i].id.model)
      return true;
  }
  return false;
}

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
