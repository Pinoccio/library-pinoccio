/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_SERVO_MODULE_H_
#define LIB_PINOCCIO_SERVO_MODULE_H_

#include "../Module.h"

namespace pinoccio {
  class ServoModule : public Module {

    public:
      bool enable();
      const __FlashStringHelper *name() const;
      void loop();
      static ServoModule instance;

    private:
      using Module::Module;
  };
}

#endif