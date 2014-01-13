#ifndef LIB_PINOCCIO_BACKPACK_H_
#define LIB_PINOCCIO_BACKPACK_H_

#include <Pinoccio.h>

class PinoccioBackpack {

  public:
    PinoccioBackpack();
    ~PinoccioBackpack();

    void setup();
    void loop();

    void dumpBackpacks();
    void printHex(const uint8_t *buf, uint8_t len);

  protected:
    uint16_t family;
    uint32_t id;
};

typedef PinoccioBackpack Backpack;

#endif