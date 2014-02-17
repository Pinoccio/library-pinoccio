// Class for parsing Pinoccio Backpack Bus Protocol EEPROM contents
#ifndef LIB_PINOCCIO_PBBP_EEPROM_H
#define LIB_PINOCCIO_PBBP_EEPROM_H

#include <stdint.h>
#include <stdlib.h>
#include "utility/endian_integer.h"
#include "PBBP.h"
#include "Minifloat.h"
#include "static_assert.h"

class Pbbe {
public:
  struct Eeprom {
    uint8_t size;
    uint8_t raw[];
  };

  static const uint16_t EEPROM_CRC_POLY = 0xa7d3;

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
  static_assert(sizeof(UniqueId) == PBBP::UNIQUE_ID_LENGTH, "Mismatching unique id length?");

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

  enum DescriptorType {
    DT_RESERVED = 0x0,
    DT_GROUP = 0x1,
    DT_POWER_USAGE = 0x2,
    DT_DATA = 0x3,
    DT_IOPIN = 0x4,
    DT_UART = 0x5,
    DT_I2C_SLAVE = 0x6,
    DT_SPI_SLAVE = 0x7,
    DT_EMPTY = 0xff,
  };

  typedef uint8_t PhysicalPin;

  struct Descriptor { };

  struct spi_speed_t : Minifloat<4, 4, 6> {
    using Minifloat::operator=;
  };

  struct SpiSlaveDescriptor : Descriptor {
    PhysicalPin ss_pin;
    /** Speed in Mhz */
    spi_speed_t speed;
    char name[];
  };

  struct UartDescriptor : Descriptor {
    PhysicalPin tx_pin;
    PhysicalPin rx_pin;
    /* Speed in bps */
    uint32_t speed;
    char name[];
  };

  struct IoPinDescriptor : Descriptor {
    PhysicalPin pin;
    char name[];
  };

  struct GroupDescriptor : Descriptor {
    char name[];
  };

  struct power_usage_t : Minifloat<4, 4, -4> {
    using Minifloat::operator=;
  };

  struct PowerUsageDescriptor : Descriptor {
    PhysicalPin power_pin;
    /* Power usages in μA */
    power_usage_t minimum;
    power_usage_t typical;
    power_usage_t maximum;
  };

  struct I2cSlaveDescriptor : Descriptor {
    /** 7-bit I2c address excluding read/write bit */
    uint8_t addr;
    /** Speed in kbit/s */
    uint16_t speed;
    char name[];
  };

  struct DataDescriptor : Descriptor {
    uint8_t length;
    // Points to memory after name, so do not separately free this pointer
    uint8_t *data;
    uint8_t name;
  };

  struct DescriptorInfo {
    DescriptorType type;
    /** Offset within the EEPROM */
    size_t offset;
    /** Group that this descriptor belongs to (NULL for DT_GROUP) */
    DescriptorInfo *group;
    /** Parsed version of this descriptor */
    Descriptor *parsed;
  };

  struct DescriptorList {
    uint8_t num_descriptors;
    DescriptorInfo info[];
  };

  /**
   * Returns a list of descriptors from the EEPROM in newly allocated
   * memory.
   *
   * Does not actually parse the individual descriptors, so the `parsed`
   * member of each DescriptorInfo is empty.
   *
   * @param eep       The EEPROM to parse.
   * @param header    Previously parsed header, if available.
   *
   * @returns a pointer to newly allocated memory containing the list of
   * descriptors. If the complete list of descriptors could not be
   * parsed, returns NULL.
   *
   * It is the responsibility of the caller to free the returned pointer
   * using free().
   */
  static DescriptorList *parseDescriptorListA(const Eeprom *eep, const Header *h = NULL);

  /**
   * Parse a single descriptor into newly allocated memory and store the
   * result into the given DescriptorInfo (in the parsed member).
   *
   * @param eep       The EEPROM to parse.
   * @param info      The descriptorInfo describing the descriptor to be
   *                  parsed (as previously returned by parseDescriptorListA).
   *
   * @returns true when parsing was succesful, false otherwise.
   *
   * When true is returend a pointer to newly allocated memory
   * containing the parsed descriptor is stored in info->parsed.
   * To use this pointer, it should be cast to the Descriptor subclass
   * corresponding to info->type.
   *
   * It is the responsibility of the caller to free the info->parsed
   * pointer using free().
   */
  static bool parseDescriptorA(const Eeprom *eep, DescriptorInfo *info);

  /**
   * Parse the header of the EEPROM, up to the the first descriptor and
   * return in in newly allocated memory.
   *
   * @param eep       The EEPROM to parse.
   *
   * @returns a pointer to newly allocated memory containing the header.
   * If no valid header could be parsed, returns NULL.
   *
   * It is the responsibility of the caller to free the returned pointer
   * using free().
   */
  static Header *parseHeaderA(const Eeprom *eep);

  /**
   * Parse a string and return it in newly allocated memory.
   *
   * @param eep       The EEPROM to parse.
   * @param offset    The offset of the first character of the string.
   *
   * @returns a pointer to newly allocated memory, containing the parsed
   * and zero-terminated string. If no valid string was found, returns
   * NULL.
   *
   * It is the responsibility of the caller to free the returned pointer
   * using free().
   */
  static char *parseStringA(const Eeprom *eep, size_t offset);

  /**
   * Find out the length of a string in the EEPROM.
   *
   * @param eep       The EEPROM to parse.
   * @param offset    The offset of the first character of the string.
   *
   * @returns the number of characters (ASCII bytes) in the string, or 0
   * if no valid string was found.
   */
  static size_t stringLength(const Eeprom *eep, size_t offset);

  /**
   * Extract a string from EEPROM into a preallocated buffer, if you
   * already know how long it will be.
   *
   * Be careful when using this function, since it does not check if you
   * aren't reading beyond the end of the string or EEPROM if you pass
   * in the wrong size!
   *
   * @param eep       The EEPROM to parse.
   * @param offset    The offset of the first character of the string.
   * @param str       The buffer into which to put the string. Should be
   *                  strlen + 1 (!) long.
   * @param strlen    The length of the string in EEPROM, as returned by
   *                  stringLength.
   */
  static void extractString(const Eeprom *eep, size_t offset, char *str, size_t strlen);

  struct MajorMinor {
    uint8_t major : 4;
    uint8_t minor : 4;
  };

  /**
   * Split a hardware revision into a major and minor part.
   */
  static MajorMinor extractMajorMinor(uint8_t rev) { return {rev >> 4, rev & 0xf}; }

  /**
   * Read the EEPROM from a slave into a newly allocated buffer.
   *
   * @param pbbp     The PBBP instance to use to talk to the slave.
   * @param addr     The backpack bus address of the slave.
   *
   * @returns a pointer to newly allocated memory containing the EEPROM
   * contents. If the EEPROM could be retrieved, or the checksum fails,
   * returns NULL.
   *
   * It is the responsibility of the caller to free the returned pointer
   * using free().
   */
  static Eeprom *getEeprom(PBBP &pbpp, uint8_t addr);

  /**
   * Write the EEPROM in the given slave.
   *
   * @param pbbp     The PBBP instance to use to talk to the slave.
   * @param addr     The backpack bus address of the slave.
   * @param eeprom   The EEPROM content to write to the slave.
   *
   * @returns true when the EEPROM contents were valid and could be
   *          succesfully written, false otherwise.
   */
  static bool writeEeprom(PBBP &pbbp, uint8_t addr, const Eeprom *eeprom);

  /**
   * Update part of the given eeprom with the given bytes.
   *
   * To enlarge the EEPROM used size, simply update bytes paste the
   * currend used size. To shrink it, replace the last descriptor(s)
   * with 0xff bytes. Automatically updates the used size in the EEPROM
   * header and recalculates the checksum.
   *
   * @param eeprom     The current eeprom contents
   * @param offset     The offset of the bytes to update
   * @param buf        The new bytes to put at offset. Should be length
   *                   bytes long.
   * @param length     The number of bytes to replace. Should not
   *                   include the checksum bytes.
   *
   * @returns The update EEPROM contents. If the size has changed, the
   *          memory might be realloc'd and this pointer could be
   *          different from the one passed in. If the EEPROM contents
   *          could not be updated, NULL is returned.
   */
  static Eeprom *updateEeprom(Eeprom *eep, size_t offset, const uint8_t *buf, uint8_t length);

  /**
   * Calculate the checksum for a unique ID.
   *
   * @param buf      A pointer to the raw bytes of the unique id. Should
   *                 be (at least) UNIQUE_ID_LENGTH - 1 bytes long.
   *
   * @returns The checksum of the unique id passed.
   */
  static uint8_t uniqueIdChecksum(uint8_t *buf);

  /**
   * Calculate the checksum for an EEPROM.
   *
   * @param buf      A pointer to the raw bytes of the eeprom. Should
   *                 be length bytes long.
   *
   * @param length   The length of the EEPROM passed. This should not
   *                 include the checksum bytes itself, since the
   *                 checksum is calculated using all length bytes.
   *
   * @returns The checksum of the eeprom passed.
   */
  static uint16_t eepromChecksum(uint8_t *buf, size_t length);

  static bool isReadonly(const Eeprom *eep, size_t offset);

protected:
  struct MinimalHeader {
    uint8_t layout_version;
    /** Header length, excluding the name */
    uint8_t header_length;
    /** Length of the name */
    uint8_t name_length;
  };

  /** Parse only basic attributes from the header (in particular, don't
   *  fully parse the variable sized name, only find out how long it
   *  would be.
   *
   * @param eep       The EEPROM to parse.
   * @param h         The MinimalHeader to fill.
   */
  static bool parseMinimalHeader(const Eeprom *eep, MinimalHeader *h);

  struct MinimalDescriptor {
    DescriptorType type;
    /* Length of descriptor, excluding any name */
    uint8_t descriptor_length;
    /* Length of name, if any */
    uint8_t name_length;
  };

  /** Parse only basic attributes from a descriptor (in particular, don't
   *  fully parse the variable sized name, only find out how long it
   *  would be.
   *
   * @param eep       The EEPROM to parse.
   * @param offset    The offset of the descriptor's first byte (== type).
   * @param d         The MinimalDescriptor to fill.
   */
  static bool parseMinimalDescriptor(const Eeprom *eep, size_t offset, MinimalDescriptor *d);
};

#endif // LIB_PINOCCIO_PBBP_EEPROM_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
