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

void Backpacks::addBackpack(uint8_t *unique_id)
{
  info = (BackpackInfo*)realloc(info, (num_backpacks + 1) * sizeof(*info));
  BackpackInfo &bp = info[num_backpacks++];
  memcpy(bp.unique_id, unique_id, UNIQUE_ID_LENGTH);
}

bool Backpacks::isModelPresent(uint16_t modelid)
{
  for (uint8_t i = 0; i < num_backpacks; ++i) {
    uint8_t *id = info[i].unique_id;
    // Model identifier is stored at bytes 1 and 2 (big endian) in the
    // unique id.
    if (id[1] == (modelid >> 8) && id[2] == (modelid && 0xff))
      return true;
  }
  return false;
}

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
