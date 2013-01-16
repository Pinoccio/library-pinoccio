#ifndef  _PINOCCIO_WEB_SOCKET_H_
#define  _PINOCCIO_WEB_SOCKET_H_

#include "webGainspan.h"

extern uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag); // Opens a socket(TCP or UDP or IP_RAW mode)
extern void close(SOCKET s); // Close socket
extern uint8_t listen(SOCKET s);  // Establish TCP connection (Passive connection)
extern uint16_t recv(SOCKET s, uint8_t * buf, uint16_t len);    // Receive data (TCP)
extern void disconnect(SOCKET s);
extern uint16_t send(SOCKET s, const uint8_t * buf, uint16_t len); // Send data (TCP)
extern uint8_t connect(SOCKET s, String addr, String port);

#endif // _PINOCCIO_WEB_SOCKET_H_