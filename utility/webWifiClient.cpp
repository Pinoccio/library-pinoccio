#include "webGainspan.h"
#include "webSocket.h"

extern "C" {
  #include "string.h"
}

#include <Arduino.h>
#include <Pinoccio.h>

#include "webWifi.h"
#include "webWifiClient.h"
#include "webWifiServer.h"

uint16_t PinoccioWifiClient::_srcport = 1024;

PinoccioWifiClient::PinoccioWifiClient() : _sock(MAX_SOCK_NUM) {
  _protocol = IPPROTO::TCP;
}

PinoccioWifiClient::PinoccioWifiClient(uint8_t sock) : _sock(sock) {
  _protocol = IPPROTO::TCP;
}


int PinoccioWifiClient::connect(const char* host, uint16_t port) {
  /* TODO */
  return 0;

  // Look up the host first
 /*
  int ret = 0;
   DNSClient dns;
   IPAddress remote_addr;

   dns.begin(Ethernet.dnsServerIP());
   ret = dns.getHostByName(host, remote_addr);
   if (ret == 1) {
     return connect(remote_addr, port);
   } else {
     return ret;
   }*/

}

int PinoccioWifiClient::connect(IPAddress ip, uint16_t port, uint8_t protocol) {
  _protocol = protocol;
  return PinoccioWifiClient::connect(ip, port);
}


int PinoccioWifiClient::connect(IPAddress ip, uint16_t port) {
  String ipAddress = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
  String ipPort = String(port);

  D(Serial.println("DEBUG: PinoccioWifiClient::connect 1"));

  if (_sock != MAX_SOCK_NUM) {
    D(Serial.println("DEBUG: PinoccioWifiClient::connect 1.1"));
    return 0;
  }

  D(Serial.println("DEBUG: PinoccioWifiClient::connect 2"));

  for (int i = 0; i < MAX_SOCK_NUM; i++) {
    if (Gainspan.readSocketStatus(i) == SOCK_STATUS::CLOSED) {
      _sock = i;
      break;
    }
  }

  D(Serial.print("DEBUG: PinoccioWifiClient::connect socket: "));
  D(Serial.println(_sock));

  D(Serial.println("DEBUG: PinoccioWifiClient::connect 3"));

  if (_sock == MAX_SOCK_NUM) {
    D(Serial.println("DEBUG: PinoccioWifiClient::connect 3.1"));
    return 0;
  }

  _srcport++;
  if (_srcport == 0) _srcport = 1024;
  D(Serial.println("DEBUG: PinoccioWifiClient::connect 4"));

  if (_protocol == IPPROTO::TCP) {
      socket(_sock, IPPROTO::TCP, _srcport, 0);
  } else if (_protocol == IPPROTO::UDP) {
      socket(_sock, IPPROTO::UDP_CLIENT, _srcport, 0);
  }
  D(Serial.println("DEBUG: PinoccioWifiClient::connect 5"));

  if (!::connect(_sock, ipAddress, ipPort)) {
    D(Serial.println("DEBUG: PinoccioWifiClient::connect 5.1"));
    _sock = MAX_SOCK_NUM;
    return 0;
  }

  D(Serial.println("DEBUG: PinoccioWifiClient::connect 6"));
  while (status() != SOCK_STATUS::ESTABLISHED) {
    delay(1);
    if (status() == SOCK_STATUS::CLOSED) {
      D(Serial.println("DEBUG: PinoccioWifiClient::connect 6.1"));
      _sock = MAX_SOCK_NUM;
      return 0;
    }
  }

  D(Serial.println("DEBUG: PinoccioWifiClient::connect 7"));
  return 1;
}

uint8_t PinoccioWifiClient::status() {
  if (_sock == MAX_SOCK_NUM) {
    return SOCK_STATUS::CLOSED;
  }
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
  //D(Serial.println("DEBUG: PinoccioWifiClient::available 1"));
  if (_sock != MAX_SOCK_NUM) {
    //D(Serial.println("DEBUG: PinoccioWifiClient::available 2"));
    if (Gainspan.isDataOnSock(_sock)) {
      //D(Serial.println("DEBUG: PinoccioWifiClient::available 2.1"));
      return 1;
    } else {
      //D(Serial.println("DEBUG: PinoccioWifiClient::available 2.2"));
      Gainspan.process();
      //D(Serial.println("DEBUG: PinoccioWifiClient::available 2.3"));
      return 0;
    }
  }
  //D(Serial.println("DEBUG: PinoccioWifiClient::available 3"));
  return 0;
}

int PinoccioWifiClient::read() {
  uint8_t b = 0;
  if (!available() || (recv(_sock, &b, 1) != 1)) {
    return -1;
  }
  return b;
}

int PinoccioWifiClient::read(uint8_t *buf, size_t size) {
  return recv(_sock, buf, size);
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