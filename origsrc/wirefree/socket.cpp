/*
socket.cpp - network sockett class 

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

#include "socket.h"
#include <Wirefree.h>

static uint16_t local_port;

/**
 * @brief	This Socket function initialize the channel in perticular mode, and set the port and wait for W5100 done it.
 * @return 	1 for success else 0.
 */
uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag)
{
  //uint8_t ret;
  //if ((protocol == IPPROTO::TCP))
  //{
    close(s);
    if (port != 0) {
      GS.configSocket(s, protocol, port);
    } 
    else {
      local_port++; // if don't set the source port, set local_port number.
      GS.configSocket(s, protocol, local_port);
    }

    //W5100.execCmdSn(s, Sock_OPEN);
    
    return 1;
  //}

  //return 0;
}


/**
 * @brief	This function close the socket and parameter is "s" which represent the socket number
 */
void close(SOCKET s)
{
  GS.execSocketCmd(s, CMD_CLOSE_CONN);
}


/**
 * @brief	This function established  the connection for the channel in passive (server) mode. This function waits for the request from the peer.
 * @return	1 for success else 0.
 */
uint8_t listen(SOCKET s)
{
  if (GS.readSocketStatus(s) != SOCK_STATUS::INIT)
    return 0;
  if (GS.getSocketProtocol(s) == IPPROTO::TCP) {
    GS.execSocketCmd(s, CMD_TCP_LISTEN);
  } else if (GS.getSocketProtocol(s) == IPPROTO::UDP) {
    GS.execSocketCmd(s, CMD_UDP_LISTEN);
  }
  return 1;
}

uint16_t recv(SOCKET s, uint8_t *buf, uint16_t len)
{
  uint16_t ret=0;

  if ( (len > 0) && GS.isDataOnSock(s))
  {
    ret = GS.readData(s, buf, len);
  }

  return ret;
}

void disconnect(SOCKET s)
{
  GS.execSocketCmd(s, CMD_CLOSE_CONN);
}

uint16_t send(SOCKET s, const uint8_t * buf, uint16_t len)
{
	uint8_t status=0;
	uint16_t ret=0;
	uint16_t freesize=0;
	
	if (len > GS.SSIZE) 
		ret = GS.SSIZE; // check size not to exceed MAX size.
	else 
		ret = len;
	
	ret = GS.writeData(s, buf, len);
	
	return ret;
}

/**
 * @brief	This function established  the connection for the channel in Active (client) mode. 
 * 		This function waits for the untill the connection is established.
 * 		
 * @return	1 for success else 0.
 */
uint8_t connect(SOCKET s, String addr, String port)
{
	String ip;
	
	ip = addr;  // FIXME : why local variable?
	if 
		(
		 ((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
		 ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) //||
		 //(port == 0x00)
		 ) 
		return 0;
	
	// set destination IP
	return GS.connectSocket(s, ip, port);
}
