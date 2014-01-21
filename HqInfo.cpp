#include "HqHandler.h"

//#define USE_TLS


const char HqHandler::host[] = "api.pinocc.io";
#ifndef USE_TLS
// 22757 for TLS, 22756 for plain
const uint16_t HqHandler::port = 22756;
const uint8_t HqHandler::cacert[] = {};
const size_t HqHandler::cacert_len = 0;
#else
// 22757 for TLS, 22756 for plain
const uint16_t HqHandler::port = 22757;

// CA certificate that signed the server certificate.
//  - Using the server certificate here doesn't work, only the CA that
//    signed it is checked (except self-signed certificates where the
//    CA and server certificates are the same, though this was not
//    tested).
//  - No checks of the server certificate (like hostname) are done,
//    other than to confirm that it was indeed signed by the right CA.
//    This means that if you use a server certificate signed by a
//    commercial CA, _any_ other certificate signed by the same CA
//    will also pass the check, which is probably not what you want...
//  - This should be a certificate in (binary) DER format. To convert
//    it to something that can be pasted below, you can use the
//    `xxd -i` command, which should be available on Linux and MacOS X.
//
const uint8_t HqHandler::cacert[] = {
  //TODO
};
const size_t HqHandler::cacert_len = sizeof(cacert);
#endif
