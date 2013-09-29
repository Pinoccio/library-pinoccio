/*
WifiServer.h - network server class 

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

#ifndef _wifi_server_h_
#define _wifi_server_h_

#include "Print.h"

class WifiClient;

class WifiServer : 
public Print {
private:
  uint16_t _port;
  uint8_t protocol;
  void accept();
public:
  WifiServer(uint16_t, uint8_t);
  WifiClient available();
  void begin();
  virtual size_t write(uint8_t);
  virtual size_t write(const char *str);
  virtual size_t write(const uint8_t *buf, size_t size);
};

#endif // _wifi_server_h_

