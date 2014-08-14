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

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

/**
 * This class handles direct connections to the HQ server (e.g., through
 * wifi).
 */
class HqHandler {
public:

  // TODO: Move more code into here.

  /////////////////////////////////////////
  // These are defined in HQInfo.cpp
  /////////////////////////////////////////

  /** The CA certificate for the hq server. */
  static const uint8_t cacert[];
  /** The length of cacert. Is 0 when TLS should not be used. */
  static const size_t cacert_len;

  static const String& host() { return _host; }
  static uint16_t port() { return _port; }
  static bool use_tls() { return _use_tls; }

  /**  Change the HQ address to connect to. */
  static void setHqAddress(const String& host, bool use_tls = false, uint16_t port = 0);

protected:
  /** Should tls be used? */
  static bool _use_tls;
  /** Hostname of the hq server */
  static String _host;
  /** Port of the hq server */
  static uint16_t _port;
};

#endif // LIB_PINOCCIO_HQHANDLER_H_
