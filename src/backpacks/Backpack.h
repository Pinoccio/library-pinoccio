/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_BACKPACK_H_
#define LIB_PINOCCIO_BACKPACK_H_

#include <Pinoccio.h>

class BackpackInfo;

namespace pinoccio {
  class Backpack {
    public:
      virtual bool setup(BackpackInfo *info) = 0;
      virtual void loop() = 0;
  };
} // namespace pinoccio

#endif
