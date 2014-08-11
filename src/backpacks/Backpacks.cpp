/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include "Backpacks.h"
#include "Scout.h"

using namespace pinoccio;

uint8_t Backpacks::num_backpacks = 0;
BackpackInfo *Backpacks::info = NULL;
PBBP Backpacks::pbbp;
Pbbe::LogicalPin::mask_t Backpacks::used_pins = 0;

void Backpacks::setup()
{
  // Give the slaves on the backpack bus a bit of time to start up. 1ms
  // seems to be enough, but let's be generous.
  delay(5);
  pbbp.begin(BACKPACK_BUS);
  detect();

  // load modules based on their backpack model
  if(Backpacks::isModelPresent(0x0001)) ModuleHandler::load("wifi");

}

void Backpacks::loop()
{
}

bool Backpacks::detect()
{
  free(info);
  num_backpacks = 0;
  used_pins = 0;
  if (!pbbp.enumerate(addBackpack))
    return printPbbpError("Backpack enumeration failed: ");
  updateUsedPins();
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
      freeAllDescriptors();
      return NULL;
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

Pbbe::LogicalPin::mask_t BackpackInfo::getUsedPins() {
  if (this->used_pins != USED_PINS_UNKNOWN)
    return this->used_pins;

  Pbbe::LogicalPin::mask_t used = 0;

  Pbbe::DescriptorList *list = getAllDescriptors();
  for (uint8_t i = 0; i < list->num_descriptors; ++i) {
    Pbbe::DescriptorInfo& info = list->info[i];

    switch (info.type) {
      case Pbbe::DT_GROUP: {
	break;
      }
      case Pbbe::DT_POWER_USAGE: {
	Pbbe::PowerUsageDescriptor& d = static_cast<Pbbe::PowerUsageDescriptor&>(*info.parsed);
	used |= d.power_pin.logical().mask();
	break;
      }
      case Pbbe::DT_DATA: {
	break;
      }
      case Pbbe::DT_IOPIN: {
	Pbbe::IoPinDescriptor& d = static_cast<Pbbe::IoPinDescriptor&>(*info.parsed);
	used |= d.pin.logical().mask();
	break;
      }
      case Pbbe::DT_UART: {
	Pbbe::UartDescriptor& d = static_cast<Pbbe::UartDescriptor&>(*info.parsed);
	used |= d.tx_pin.logical().mask();
	used |= d.rx_pin.logical().mask();
	break;
      }
      case Pbbe::DT_I2C_SLAVE: {
	used |= Pbbe::LogicalPin(SCL).mask();
	used |= Pbbe::LogicalPin(SDA).mask();
	break;
      }
      case Pbbe::DT_SPI_SLAVE: {
	Pbbe::SpiSlaveDescriptor& d = static_cast<Pbbe::SpiSlaveDescriptor&>(*info.parsed);
	used |= d.ss_pin.logical().mask();
	used |= Pbbe::LogicalPin(MISO).mask();
	used |= Pbbe::LogicalPin(MOSI).mask();
	used |= Pbbe::LogicalPin(SCK).mask();
	break;
      }
      default: {
	// Should not occur
	break;
      }
    }
  }

  this->used_pins = used;
  return used;
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
  bp.used_pins = BackpackInfo::USED_PINS_UNKNOWN;

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

void Backpacks::updateUsedPins()
{
  Pbbe::LogicalPin::mask_t used = 0;
  for (uint8_t i = 0; i < num_backpacks; ++i) {
    used |= info[i].getUsedPins();
  }

  used_pins = used;
}

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
