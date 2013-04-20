#ifndef LIB_PINOCCIO_BACKPACK_H_
#define LIB_PINOCCIO_BACKPACK_H_

#include "Pinoccio.h"

class PinoccioBackpack {

  public:
    PinoccioBackpack();
    ~PinoccioBackpack();

    void init();
    void loop();
    
  protected:
    uint_16 family;
    uint_32 id;
};

#endif