/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_OTA_MODULE_H_
#define LIB_PINOCCIO_OTA_MODULE_H_

#include "../Module.h"

namespace pinoccio {
  class OtaModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();

    protected:

    // Ensure there is always exactly one instance by declaring it here
    // and making our constructor private
    private:
      using Module::Module;
    public:
      static OtaModule instance;
  };
} // namespace pinoccio

#endif
