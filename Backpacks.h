// Class that collects info about connected backpacks
#ifndef LIB_PINOCCIO_BACKPACKS_H
#define LIB_PINOCCIO_BACKPACKS_H

#include "PBBP.h"
#include "Pbbe.h"

/**
 * Structure collecting some info on a backpack. Do not create any of
 * these objects outside of Backpacks::info, since getAddress() only
 * works when this struct lives inside that array.
 */
struct BackpackInfo {
  Pbbe::UniqueId id;

  /**
   * Retrieve the Eeprom a backpack, if not already done so.
   *
   * @param len     Output parameter that gets set to the EEPROM used
   *                size (e.g., the length of the buffer returned. Can
   *                be NULL to ignore the length.
   * @returns The value of eeprom_contents if succesful, NULL otherwise.
   */
  Pbbe::Eeprom* getEeprom(size_t *len = NULL);

  /**
   * Free the memory used by the eeprom contents.
   */
  void freeEeprom();

  /**
   * Gets the parsed EEPROM header of this backpack, or NULL if it could
   * not be parsed.
   */
  Pbbe::Header *getHeader();

  /**
   * Free the memory used by the header.
   */
  void freeHeader();

  /**
   * Returns the bus address of this backpack.
   */
  uint8_t getAddress();

  /**
   * Returns a list of all parsed descriptors, or NULL if not everything
   * could be parsed.
   *
   * All the "parsed" members of teach DescriptorInfo in the list is
   * already filled.
   */
  Pbbe::DescriptorList *getAllDescriptors();

  /**
   * Free the entire list of descriptors.
   */
  void freeAllDescriptors();

  Pbbe::Eeprom *eep;
  Pbbe::Header *header;
  Pbbe::DescriptorList *descriptors;
protected:
  // Declare a private constructor to prevent people from allocating new
  // BackpackInfo objects outside of Backpacks::info (which would break
  // getAddress).
  BackpackInfo() {}

  friend class Backpacks;
};

class Backpacks {
public:
  /**
   * Autodetect connected backpacks.
   */
  static bool detect();

  static void setup();
  static void loop() {}

  /**
   * See if a backpack with the given model identifier is present.
   */
  static bool isModelPresent(uint16_t modelid);

  static uint8_t num_backpacks;
  static BackpackInfo *info;
  static PBBP pbbp;

protected:
  /**
   * Add a backpack
   */
  static void addBackpack(uint8_t *unique_id);

  /**
   * Print a Pbbp error. Prints the given prefix, followed by the
   * last pbbp error.
   *
   * @returns false
   */
  static bool printPbbpError(const char *prefix);

  friend class BackpackInfo;
};

#endif // LIB_PINOCCIO_BACKPACKS_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
