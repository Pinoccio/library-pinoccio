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

  /**
   * Struct containing parsed info from the EEPROM header, as returned
   * by parseHeaderA.
   *
   * Does _not_ represent the actual layout of the the header in EEPROM
   * exactly, so don't use it to map the EEPROM contents.
   */
  struct Header {
    uint8_t layout_version;
    uint8_t total_eeprom_size;
    uint8_t used_eeprom_size;
    /* Unique ID is not present, since that is expected to be stored
     * somewhere else during enumeration */
    uint8_t firmware_version;
    /** Offset of the first descriptor. */
    uint8_t descriptor_offset;
    /**
     * Backpack name. Intended to be allocated to the correct length by
     * adding the string length plus nul terminator to the allocation
     * size of the struct. */
    char backpack_name[];
  };

  /**
   * Parse the header of the EEPROM, up to the the first descriptor and
   * return in in newly allocated memory.
   *
   * @param buf       A buffer containing the EEPROM.
   * @param len       The size of the EEPROM contents (e.g., length of
   *                  buf).
   *
   * @returns a pointer to newly allocated memory containing the header.
   * If no valid header could be parsed, returns NULL.
   *
   * It is the responsibility of the caller to free the returned pointer
   * using free().
   */
  static Header *parseHeaderA(const uint8_t *buf, size_t len);

  /**
   * Parse a string and return it in newly allocated memory.
   *
   * @param buf       The part of the EEPROM that contains the unparsed
   *                  string. Should point to the first character of the
   *                  string.
   * @param len       The remaining valid size of the EEPROM contents,
   *                  starting from buf.
   *
   * @returns a pointer to newly allocated memory, containing the parsed
   * and zero-terminated string. If no valid string was found, returns
   * NULL.
   *
   * It is the responsibility of the caller to free the returned pointer
   * using free().
   */
  static char *parseStringA(const uint8_t *buf, size_t len);

  /**
   * Find out the length of a string in the EEPROM.
   *
   * @param buf       The part of the EEPROM that contains the unparsed
   *                  string. Should point to the first character of the
   *                  string.
   * @param len       The remaining valid size of the EEPROM contents,
   *                  starting from buf.
   *
   * @returns the number of characters (ASCII bytes) in the string, or 0
   * if no valid string was found.
   */
  static size_t stringLength(const uint8_t *buf, size_t len);

  /**
   * Extract a string from EEPROM into a preallocated buffer, if you
   * already know how long it will be.
   *
   * Be careful when using this function, since it does not check if you
   * aren't reading beyond the end of the string or EEPROM if you pass
   * in the wrong size!
   *
   * @param buf       The part of the EEPROM that contains the unparsed
   *                  string. Should point to the first character of the
   *                  string.
   * @param str       The buffer into which to put the string. Should be
   *                  strlen + 1 (!) long.
   * @param strlen    The length of the string in EEPROM, as returned by
   *                  stringLength. This is _not_ the length of the
   *                  buffer and _not_ the length of the entire eeprom
   *                  contents.
   */
  static void extractString(const uint8_t *buf, char *str, size_t strlen);

  struct MajorMinor {
    uint8_t major : 4;
    uint8_t minor : 4;
  };

  /**
   * Split a hardware revision into a major and minor part.
   */
  static MajorMinor extractMajorMinor(uint8_t rev) { return {rev >> 4, rev & 0xf}; }
};

#endif // LIB_PINOCCIO_PBBP_EEPROM_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
