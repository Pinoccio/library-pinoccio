/*
WifiServer.cpp - network server class 

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

//extern "C" {
//#include "string.h"
//}

#include "Wirefree.h"
#include "WifiClient.h"
#include "WifiServer.h"

WifiServer::WifiServer(uint16_t port, uint8_t proto)
{
  _port = port;
  protocol = proto;
}

void WifiServer::begin()
{
  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    WifiClient client(sock);
    if (client.status() == SOCK_STATUS::CLOSED) {
      if (protocol == IPPROTO::TCP) {
        socket(sock, IPPROTO::TCP, _port, 0);
      } else if (protocol == IPPROTO::UDP) {
        socket(sock, IPPROTO::UDP, _port, 0);
      }
      listen(sock);
      Wirefree::_server_port[sock] = _port;
      break;
    }
  }
}

size_t WifiServer::write(uint8_t b)
{
}

size_t WifiServer::write(const char *str)
{
}

size_t WifiServer::write(const uint8_t *buffer, size_t size)
{
}

void WifiServer::accept()
{
  int listening = 0;

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    WifiClient client(sock);

    if (Wirefree::_server_port[sock] == _port) {
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

WifiClient WifiServer::available()
{
    GS.process();

  accept();

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    WifiClient client(sock);
    if (/*Wirefree::_server_port[sock] == _port &&*/
        (client.status() == SOCK_STATUS::ESTABLISHED ||
         client.status() == SOCK_STATUS::CLOSE_WAIT))
    {
      if (client.available()) {
        // XXX: don't always pick the lowest numbered socket.
        return client;
      }
    }
  }

  return WifiClient(MAX_SOCK_NUM);
}

#if 0
void WifiServer::write(uint8_t b) 
{
  write(&b, 1);
}

void WifiServer::write(const char *str) 
{
  write((const uint8_t *)str, strlen(str));
}

void WifiServer::write(const uint8_t *buffer, size_t size) 
{
  accept();

  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    WifiClient client(sock);

    if (EthernetClass::_server_port[sock] == _port &&
      client.status() == SnSR::ESTABLISHED) {
      client.write(buffer, size);
    }
  }
}
#endif

