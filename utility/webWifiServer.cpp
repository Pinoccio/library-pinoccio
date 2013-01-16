#include "webGainspan.h"
#include "webSocket.h"

#include "webWifi.h"
#include "webWifiClient.h"
#include "webWifiServer.h"

PinoccioWifiServer::PinoccioWifiServer(uint16_t port, uint8_t proto)
{
  _port = port;
  protocol = proto;
}

void PinoccioWifiServer::begin()
{
  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    PinoccioWifiClient client(sock);
    if (client.status() == SOCK_STATUS::CLOSED) {
      if (protocol == IPPROTO::TCP) {
        socket(sock, IPPROTO::TCP, _port, 0);
      } else if (protocol == IPPROTO::UDP) {
        socket(sock, IPPROTO::UDP, _port, 0);
      }
      listen(sock);
      webWifi::_server_port[sock] = _port;
      break;
    }
  }
}

size_t PinoccioWifiServer::write(uint8_t b)
{
}

size_t PinoccioWifiServer::write(const char *str)
{
}

size_t PinoccioWifiServer::write(const uint8_t *buffer, size_t size)
{
}

void PinoccioWifiServer::accept()
{
  int listening = 0;

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    PinoccioWifiClient client(sock);

    if (webWifi::_server_port[sock] == _port) {
      if (client.status() == SOCK_STATUS::LISTEN) {
        listening = 1;
      }
      //else if (client.status() == SnSR::CLOSE_WAIT && !client.available()) {
      //  client.stop();
      //}
    }
  }

  if (!listening) {
    begin();
  }
}

PinoccioWifiClient PinoccioWifiServer::available()
{
  Gainspan.process();

  accept();

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    PinoccioWifiClient client(sock);
    if (/*webWifi::_server_port[sock] == _port &&*/
        (client.status() == SOCK_STATUS::ESTABLISHED ||
         client.status() == SOCK_STATUS::CLOSE_WAIT))
    {
      if (client.available()) {
        // XXX: don't always pick the lowest numbered socket.
        return client;
      }
    }
  }

  return PinoccioWifiClient(MAX_SOCK_NUM);
}

#if 0
void PinoccioWifiServer::write(uint8_t b)
{
  write(&b, 1);
}

void PinoccioWifiServer::write(const char *str)
{
  write((const uint8_t *)str, strlen(str));
}

void PinoccioWifiServer::write(const uint8_t *buffer, size_t size)
{
  accept();

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    PinoccioWifiClient client(sock);

    if (EthernetClass::_server_port[sock] == _port &&
      client.status() == SnSR::ESTABLISHED) {
      client.write(buffer, size);
    }
  }
}
#endif