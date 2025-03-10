/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_SCOUTHANDLER_H_
#define LIB_PINOCCIO_SCOUTHANDLER_H_

#include <Pinoccio.h>
#include <ScoutHandler.h>
#include "util/StringBuffer.h"

namespace pinoccio {
  class ScoutHandler {

    public:
      ScoutHandler();
      ~ScoutHandler();

      void setup();
      void loop();
      void announce(uint16_t group, const String& message);
      void timeSyncSend();
      bool isVerbose;
      void setVerbose(bool flag);
      StringBuffer report(StringBuffer& report);
      uint32_t seen;
      
      bool isBridged;
      StringBuffer bridge;
      void setBridged(bool flag);

    protected:
  };
} // namespace pinoccio

void leadHQConnect();

#endif
