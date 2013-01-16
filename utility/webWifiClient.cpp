#include "webGainspan.h"
#include "webSocket.h"

#include "webWifi.h"
#include "webWifiClient.h"
#include "webWifiServer.h"

uint16_t PinoccioWifiClient::_srcport = 1024;

PinoccioWifiClient::PinoccioWifiClient(uint8_t sock) : _sock(sock) {
}

PinoccioWifiClient::PinoccioWifiClient(String ip, String port, uint8_t proto) : _ip(ip), _port(port), _protocol(proto), _sock(MAX_SOCK_NUM) {
}


uint8_t PinoccioWifiClient::connect() {
  if (_sock != MAX_SOCK_NUM)
    return 0;

  for (int i = 0; i < MAX_SOCK_NUM; i++) {
    if (Gainspan.readSocketStatus(i) == SOCK_STATUS::CLOSED) {
      _sock = i;
      break;
    }
  }


  if (_sock == MAX_SOCK_NUM)
    return 0;

  _srcport++;
  if (_srcport == 0) _srcport = 1024;

  if (_protocol == IPPROTO::TCP) {
      socket(_sock, IPPROTO::TCP, _srcport, 0);
  } else if (_protocol == IPPROTO::UDP) {
      socket(_sock, IPPROTO::UDP_CLIENT, _srcport, 0);
  }

  if (!::connect(_sock, _ip, _port)) {
    _sock = MAX_SOCK_NUM;
    return 0;
  }

  while (status() != SOCK_STATUS::ESTABLISHED) {
    delay(1);
    if (status() == SOCK_STATUS::CLOSED) {
      _sock = MAX_SOCK_NUM;
      return 0;
    }
  }

  return 1;
}

uint8_t PinoccioWifiClient::status() {
  if (_sock == MAX_SOCK_NUM) return SOCK_STATUS::CLOSED;
  return Gainspan.readSocketStatus(_sock);
}

size_t PinoccioWifiClient::write(uint8_t b) {
  if (_sock != MAX_SOCK_NUM)
    send(_sock, &b, 1);

}

size_t PinoccioWifiClient::write(const char *str) {
  if (_sock != MAX_SOCK_NUM)
    send(_sock, (const uint8_t *)str, strlen(str));
}

size_t PinoccioWifiClient::write(const uint8_t *buf, size_t size) {
  if (_sock != MAX_SOCK_NUM)
    send(_sock, buf, size);
}
int PinoccioWifiClient::available() {
  if (_sock != MAX_SOCK_NUM) {
    if (Gainspan.isDataOnSock(_sock)) {
      return 1;
    } else {
      Gainspan.process();
      return 0;
    }
  }

  return 0;
}

int PinoccioWifiClient::read() {
  uint8_t b = 0;
  if (!available() || (recv(_sock, &b, 1) != 1))
    return -1;

  return b;
}

int PinoccioWifiClient::peek() {
  return 0;
}

void PinoccioWifiClient::flush() {
  while (available())
    read();
}

PinoccioWifiClient::operator bool() {
  return _sock != MAX_SOCK_NUM;
}

uint8_t PinoccioWifiClient::connected() {
  if (_sock == MAX_SOCK_NUM) return 0;

  //uint8_t s = status();
  //return !(s == SOCK_STATUS::LISTEN || s == SOCK_STATUS::CLOSED ||
  //  (s == SOCK_STATUS::CLOSE_WAIT && !available()));

  return (status() == SOCK_STATUS::ESTABLISHED);
}

void PinoccioWifiClient::stop() {
  if (_sock == MAX_SOCK_NUM)
    return;

  disconnect(_sock);

  //EthernetClass::_server_port[_sock] = 0;
  _sock = MAX_SOCK_NUM;
}