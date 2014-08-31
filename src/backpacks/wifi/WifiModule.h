/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_WIFI_MODULE_H_
#define LIB_PINOCCIO_WIFI_MODULE_H_

#include "../../modules/Module.h"
#include "WifiBackpack.h"

namespace pinoccio {
  class WifiModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();

      WifiBackpack *bp() { return _bp; }

    protected:
      WifiBackpack *_bp;

      static void onToggleBackpackVcc(bool on);

      // Internal helpers. Don't fully enable/disable the module, but
      // enable/disable the WifiBackpack part
      bool _enable();
      void _disable();

    // Ensure there is always exactly one instance by declaring it here
    // and making our constructor private
    private:
      using Module::Module;
    public:
      static WifiModule instance;
  };
} // namespace pinoccio
#endif
