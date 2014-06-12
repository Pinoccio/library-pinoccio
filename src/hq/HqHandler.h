/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_HQHANDLER_H_
#define LIB_PINOCCIO_HQHANDLER_H_

#include <stddef.h>
#include <stdint.h>
#include <Arduino.h>
#include <GS.h>
#include "../util/StringBuffer.h"

/**
 * This class handles direct connections to the HQ server (e.g., through
 * wifi).
 */
class HqHandler {
public:
  HqHandler();
  ~HqHandler();

  void setup();
  void loop();

  void announce(uint16_t group, const String& message);
  void setVerbose(bool flag);
  StringBuffer report(const String& report);

  // is the lead scout bridging to the internet
  bool isBridge();
  
  // called by wifi when network is up
  void up(GSModule gs);
  
  bool connected();
  bool available();
  
  uint16_t connCount;
  
private:
  GSTcpClient *tcp;
};

#endif // LIB_PINOCCIO_HQHANDLER_H_
