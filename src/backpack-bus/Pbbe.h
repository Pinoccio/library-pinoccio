/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
// Class for parsing Pinoccio Backpack Bus Protocol EEPROM contents
#ifndef LIB_PINOCCIO_PBBP_EEPROM_H
#define LIB_PINOCCIO_PBBP_EEPROM_H

#include <stdint.h>
#include <stdlib.h>
#include "../util/integer.h"
#include "../util/static_assert.h"

class Pbbe {
public:

  struct LogicalPin {
    LogicalPin(uint8_t val) : val(val) { }
    operator uint8_t() {return this->val; }

    // TODO: Change to use NOT_A_PIN if that's ever changed from its
    // current "0" value:
    // https://groups.google.com/a/arduino.cc/d/msg/developers/zeDXBRwW-mg/9bsG9f7Zp84J
    static const uint8_t NONE = 0xff;

    /** Bitmask for logical pins */
    typedef boost::uint_t<NUM_DIGITAL_PINS>::least mask_t;
    mask_t mask() {
      if (this->val != NONE)
        return (mask_t)1 << this->val;
      else
        return 0;
    }

    bool in(mask_t check) {
      return check & mask();
    }

    uint8_t val;
  };


protected:

};

#endif // LIB_PINOCCIO_PBBP_EEPROM_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
