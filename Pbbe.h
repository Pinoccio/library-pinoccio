// Class for parsing Pinoccio Backpack Bus Protocol EEPROM contents
#ifndef LIB_PINOCCIO_PBBP_EEPROM_H
#define LIB_PINOCCIO_PBBP_EEPROM_H

#include <stdint.h>
#include <stdlib.h>
#include "utility/endian_integer.h"

class Pbbe {
public:
  /**
   * Union describing a unique id.
   * You can either access the raw bytes using the .raw_bytes part, or the
   * separate fields using the .version, etc. fields.
   * Uses types from endian_integer.h so the memory layout matches the
   * layout used in the backpack EEPROM, using big endian numbers.
   */
  union UniqueId {
    struct {
      ubig8_t protocol_version;
      ubig16_t model;
      ubig8_t revision;
      ubig24_t serial;
      ubig8_t checksum;
    } __attribute__((packed));
    uint8_t raw_bytes[];
  };
};

#endif // LIB_PINOCCIO_PBBP_EEPROM_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
