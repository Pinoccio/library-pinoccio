#include "Pbbe.h"
#include "Arduino.h"

// Some useful constants for parsing the EEPROM

static const uint8_t CHECKSUM_SIZE = 2;

bool Pbbe::parseMinimalHeader(const Eeprom *eep, MinimalHeader *h)
{
  if (!eep || CHECKSUM_SIZE >= eep->size)
    return false;
  size_t len = eep->size - CHECKSUM_SIZE;

  if (len < 1u)
    return false;

  h->layout_version = eep->raw[0];
  h->header_length = 0xc;

  if (h->layout_version == 0 || h->layout_version > 1)
    return false;

  // Size should be at least the header + a 1-character name
  if (len < h->header_length + 1u)
    return false;

  h->name_length = stringLength(eep, h->header_length);

  if (!h->name_length)
    return false;

  return true;
}

Pbbe::Header *Pbbe::parseHeaderA(const Eeprom *eep)
{
  MinimalHeader min;

  if (!parseMinimalHeader(eep, &min))
    return NULL;

  Header *h = (Header*)malloc(sizeof(Header) + min.name_length + 1);

  h->layout_version = min.layout_version;
  h->total_eeprom_size = eep->raw[1];
  h->used_eeprom_size = eep->raw[2];
  /* Skip 8 bytes of unique id */
  h->firmware_version = eep->raw[0xb];
  extractString(eep, min.header_length, h->backpack_name, min.name_length);
  h->descriptor_offset = min.header_length + min.name_length;

  return h;
}

size_t Pbbe::stringLength(const Eeprom *eep, size_t offset) {
  // Check that we have at least one character
  if (offset + CHECKSUM_SIZE + 1 > eep->size)
    return 0;

  const uint8_t *buf = &eep->raw[offset];
  size_t len = eep->size - offset - CHECKSUM_SIZE;

  // First character is always in
  size_t strlen = 1;
  do {
    // The last character in the string will have the MSB set (which
    // makes the minimum string length 1).
    if ((buf[strlen - 1] & 0x80))
      return strlen;
  } while(strlen++ <= len);
  // Reached end of EEPROM before end of string - invalid string
  return 0;
}

void Pbbe::extractString(const Eeprom *eep, size_t offset, char *str, size_t strlen)
{
  // Add zero termination
  str[strlen] = '\0';

  // Strip the MSB from the last byte, and copy the rest as-is
  str[strlen - 1] = eep->raw[offset + strlen - 1] & 0x7f;
  while (--strlen)
    str[strlen - 1] = eep->raw[offset + strlen - 1];
}

char *Pbbe::parseStringA(const Eeprom *eep, size_t offset)
{
  size_t strlen = stringLength(eep, offset);
  char *str = (char*)malloc(strlen + 1);
  extractString(eep, offset, str, strlen);
  return str;
}

Pbbe::Eeprom *Pbbe::getEeprom(PBBP &pbbp, uint8_t addr)
{
  uint8_t buf[3];
  // Read the first 3 bytes
  if (!pbbp.readEeprom(addr, 0, buf, sizeof(buf))) {
    Serial.println("EEPROM read failed: ");
    pbbp.printLastError(Serial);
    return NULL;
  }

  // Check EEPROM version
  if (buf[0] == 0 || buf[0] > 1) {
    Serial.print("Unsupported EEPROM version: ");
    Serial.print(buf[0]);
    return NULL;
  }

  // Get the used size of the EEPROM
  uint8_t used_size = buf[2];
  // Allocate memory for that
  Eeprom *eep = (Eeprom*)malloc(sizeof(Eeprom) + used_size);
  if (!eep) {
    Serial.println("Memory allocation for EEPROM failed");
    return NULL;
  }

  eep->size = used_size;

  // And read the full EEPROM
  if (!pbbp.readEeprom(addr, 0, eep->raw, used_size)) {
    Serial.println("EEPROM read failed: ");
    pbbp.printLastError(Serial);
    free(eep);
    return NULL;
  }

  // TODO: Verify checksum

  return eep;
}


/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
