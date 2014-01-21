#ifndef LIB_PINOCCIO_HQHANDLER_H_
#define LIB_PINOCCIO_HQHANDLER_H_

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

  /** Hostname of the hq server */
  static const char host[];
  /** Port of the hq server */
  static const uint16_t port;
  /** The CA certificate for the hq server. */
  static const uint8_t cacert[];
  /** The length of cacert. Is 0 when TLS should not be used. */
  static const size_t cacert_len;
};

#endif // LIB_PINOCCIO_HQHANDLER_H_
