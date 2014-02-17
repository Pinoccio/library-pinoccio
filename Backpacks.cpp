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

bool Backpacks::detect()
{
  free(info);
  num_backpacks = 0;
  if (!pbbp.enumerate(addBackpack))
    return printPbbpError("Backpack enumeration failed: ");
  return true;
}

bool Backpacks::printPbbpError(const char *prefix)
{
  Serial.print(prefix);
  pbbp.printLastError(Serial);
  Serial.println();
  return false;
}

Pbbe::Eeprom *BackpackInfo::getEeprom(size_t *len)
{
  if (this->eep)
    return this->eep;

  this->eep = Pbbe::getEeprom(Backpacks::pbbp, getAddress());
  return this->eep;
}

void BackpackInfo::freeEeprom()
{
  free(this->eep);
  this->eep = NULL;
}

uint8_t BackpackInfo::getAddress()
{
  // Deduce our address based on our place in the Backpacks::info array.
  return this - Backpacks::info;
}

Pbbe::Header *BackpackInfo::getHeader()
{
  if (this->header)
    return this->header;

  if (!getEeprom())
    return false;

  this->header = Pbbe::parseHeaderA(this->eep);
  return this->header;
}

void BackpackInfo::freeHeader()
{
  free(this->header);
  this->header = NULL;
}

Pbbe::DescriptorList *BackpackInfo::getAllDescriptors()
{
  if (this->descriptors)
    return this->descriptors;

  if (!getEeprom())
    return NULL;

  this->descriptors = Pbbe::parseDescriptorListA(this->eep, this->header);
  if (!this->descriptors)
    return NULL;

  for (uint8_t i = 0; i < descriptors->num_descriptors; ++i) {
    Pbbe::DescriptorInfo &info = this->descriptors->info[i];
    if (!Pbbe::parseDescriptorA(this->eep, &info)) {
      //freeAllDescriptors();
      //return NULL;
    }
  }
  return this->descriptors;
}

void BackpackInfo::freeAllDescriptors() {
  if (this->descriptors) {
    for (uint8_t i = 0; i < this->descriptors->num_descriptors; ++i) {
      Pbbe::DescriptorInfo &info = this->descriptors->info[i];
      free(info.parsed);
      info.parsed = NULL;
    }
    free(this->descriptors);
    this->descriptors = NULL;
  }
}


void Backpacks::addBackpack(uint8_t *unique_id)
{
  info = (BackpackInfo*)realloc(info, (num_backpacks + 1) * sizeof(*info));
  BackpackInfo &bp = info[num_backpacks++];
  // TODO: This stuff might better belong in a constructor of
  // BackpackInfo (which is not current called!). However, doing this
  // nicely requires the placement new operator, which Arduino does not
  // (yet) supply. https://github.com/arduino/Arduino/pull/108
  bp.eep = NULL;
  bp.header = NULL;
  bp.descriptors = NULL;

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
