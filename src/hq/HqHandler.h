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
#include <telehash.h>

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

  // is the lead scout bridging to the internet
  bool isBridge();
  
  // set by last active network delivery, if any
  void setDefault(void (*net)(packet_t p));
  
  void recvPacket(packet_t p);
  
  bool connected();
  bool available();
  
  uint16_t connCount;
  
private:
  void (*onDefault)(packet_t p);
};

#endif // LIB_PINOCCIO_HQHANDLER_H_
