/*
WifiClient.cpp - network client class 

Copyright (C) 2011 DIYSandbox LLC

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "gs.h"
#include "socket.h"

#include "Wirefree.h"
#include "WifiClient.h"
#include "WifiServer.h"

uint16_t WifiClient::_srcport = 1024;

WifiClient::WifiClient(uint8_t sock) : _sock(sock) {
}

WifiClient::WifiClient(String ip, String port, uint8_t proto) : _ip(ip), _port(port), _protocol(proto), _sock(MAX_SOCK_NUM) {
}


uint8_t WifiClient::connect() {
	if (_sock != MAX_SOCK_NUM)
		return 0;
	
	for (int i = 0; i < MAX_SOCK_NUM; i++) {
		if (GS.readSocketStatus(i) == SOCK_STATUS::CLOSED) {
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

uint8_t WifiClient::status() {
  if (_sock == MAX_SOCK_NUM) return SOCK_STATUS::CLOSED;
  return GS.readSocketStatus(_sock);
}

size_t WifiClient::write(uint8_t b) {
	if (_sock != MAX_SOCK_NUM)
		send(_sock, &b, 1);

}

size_t WifiClient::write(const char *str) {
	if (_sock != MAX_SOCK_NUM)
		send(_sock, (const uint8_t *)str, strlen(str));
}

size_t WifiClient::write(const uint8_t *buf, size_t size) {
	if (_sock != MAX_SOCK_NUM)
		send(_sock, buf, size);
}

int WifiClient::available() {
	if (_sock != MAX_SOCK_NUM) {
		if (GS.isDataOnSock(_sock)) {
			return 1;
		} else {
			GS.process();
			return 0;
		}
	}

	return 0;
}

int WifiClient::read() {
  uint8_t b = 0;
  if (!available() || (recv(_sock, &b, 1) != 1))
    return -1;

  return b;
}

int WifiClient::peek() {
  return 0;
}

void WifiClient::flush() {
	while (available())
		read();
}

WifiClient::operator bool() {
  return _sock != MAX_SOCK_NUM;
}

uint8_t WifiClient::connected() {
  if (_sock == MAX_SOCK_NUM) return 0;

  //uint8_t s = status();
  //return !(s == SOCK_STATUS::LISTEN || s == SOCK_STATUS::CLOSED ||
  //  (s == SOCK_STATUS::CLOSE_WAIT && !available()));

  return (status() == SOCK_STATUS::ESTABLISHED);
}

void WifiClient::stop() {
  if (_sock == MAX_SOCK_NUM)
    return;

  disconnect(_sock);

  //EthernetClass::_server_port[_sock] = 0;
  _sock = MAX_SOCK_NUM;
}

