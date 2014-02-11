// Class that collects info about connected backpacks
#ifndef LIB_PINOCCIO_BACKPACKS_H
#define LIB_PINOCCIO_BACKPACKS_H

#include "PBBP.h"

struct BackpackInfo {
  uint8_t unique_id[UNIQUE_ID_LENGTH];
};

class Backpacks {
public:
  /**
   * Autodetect connected backpacks.
   */
  static void detect();

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
  static void addBackpack(uint8_t unique_id[UNIQUE_ID_LENGTH]);
};

#endif // LIB_PINOCCIO_BACKPACKS_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
