#include "Pbbe.h"

Pbbe::Header *Pbbe::parseHeaderA(const uint8_t *buf, size_t len)
{
  if (!buf || len < 1)
    return NULL;

  uint8_t layout_version = buf[0];

  if (layout_version == 0 || layout_version > 1)
    return NULL;

  if (len < 0xd)
    return NULL;

  const uint8_t name_offset = 0xc;

  uint8_t strlen = stringLength(buf + name_offset, len - name_offset);
  if (!strlen)
    return NULL;

  Header *h = (Header*)malloc(sizeof(Header) + strlen + 1);

  h->layout_version = layout_version;
  h->total_eeprom_size = buf[1];
  h->used_eeprom_size = buf[2];
  /* Skip 8 bytes of unique id */
  h->firmware_version = buf[0xb];
  extractString(buf + name_offset, h->backpack_name, strlen);
  h->descriptor_offset = name_offset + strlen;

  return h;
}

size_t Pbbe::stringLength(const uint8_t *buf, size_t len) {
  uint8_t strlen = 1;
  while(true) {
    if (strlen > len)
      return 0;

    // The last character in the string will have the MSB set (which
    // makes the minimum string length 1).
    if ((buf[strlen - 1] & 0x80))
      return strlen;

    strlen++;
  }
}

void Pbbe::extractString(const uint8_t *buf, char *str, size_t strlen)
{
  // Add zero termination
  str[strlen] = '\0';

  // Strip the MSB from the last byte, and copy the rest as-is
  str[strlen - 1] = buf[strlen - 1] & 0x7f;
  while (--strlen)
    str[strlen - 1] = buf[strlen - 1];
}

char *Pbbe::parseStringA(const uint8_t *buf, size_t len)
{
  size_t strlen = stringLength(buf, len);
  char *str = (char*)malloc(strlen + 1);
  extractString(buf, str, strlen);
  return str;
}

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
