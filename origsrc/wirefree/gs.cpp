/*
gs.cpp - HAL driver to talk with Gainspan GS1011 WiFi module

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

#include <stdio.h>
#include <avr/interrupt.h>
#include <HardwareSerial.h>

#include "gs.h"

GSClass GS;

/*
 * Command table definitions
 */
prog_char cmd_0[] PROGMEM = "ATE0";
prog_char cmd_1[] PROGMEM = "AT+WWPA=";
prog_char cmd_2[] PROGMEM = "AT+WA=";
prog_char cmd_3[] PROGMEM = "AT+NDHCP=0";
prog_char cmd_4[] PROGMEM = "AT+NDHCP=1";
prog_char cmd_5[] PROGMEM = "AT+WD";
prog_char cmd_6[] PROGMEM = "AT+NSTCP=";
prog_char cmd_7[] PROGMEM = "AT+NCTCP=";
prog_char cmd_8[] PROGMEM = "AT+NMAC=?";
prog_char cmd_9[] PROGMEM = "AT+DNSLOOKUP=";
prog_char cmd_10[] PROGMEM = "AT+NCLOSE=";
prog_char cmd_11[] PROGMEM = "AT+NSET=";
prog_char cmd_12[] PROGMEM = "AT+WM=2";
prog_char cmd_13[] PROGMEM = "AT+DHCPSRVR=1";
prog_char cmd_14[] PROGMEM = "AT+NSUDP=";
prog_char cmd_15[] PROGMEM = "AT+NCUDP=";

PROGMEM const char *cmd_tbl[] =
{
		cmd_0, cmd_1, cmd_2, cmd_3, cmd_4, cmd_5, cmd_6, cmd_7,
		cmd_8, cmd_9, cmd_10, cmd_11, cmd_12, cmd_13, cmd_14, cmd_15,
};

/* Make sure the cmd_buffer is large enough to hold
 * the largest command in the above table */
char cmd_buffer[16];


uint8_t hex_to_int(char c)
{
	uint8_t val = 0;

	if (c >= '0' && c <= '9') {
		val = c - '0';
	}
	else if (c >= 'A' && c <= 'F') {
		val = c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'f') {
		val = c - 'a' + 10;
	}

	return val;
}

char int_to_hex(uint8_t c)
{
	char val = '0';

	if (c >= 0 && c <= 9) {
		val = c + '0';
	}
	else if (c >= 10 && c <= 15) {
		val = c + 'A' - 10;
	}

	return val;
}

uint8_t GSClass::init()
{
	Serial.begin(9600);
	delay(1000);

	flush();
	Serial.println();
	delay(1000);

	dev_mode = DEV_OP_MODE_COMMAND;
	connection_state = DEV_CONN_ST_DISCONNECTED;
	dataOnSock = 255;

	for (int i = 0; i < 4; i++) {
	    this->sock_table[i].cid = 0;
	    this->sock_table[i].port = 0;
	    this->sock_table[i].protocol = 0;
	    this->sock_table[i].status = 0;
	}

	// disable echo
	if (!send_cmd_w_resp(CMD_DISABLE_ECHO)) {
		return 0;
	}

	// get device ID
	if (!send_cmd_w_resp(CMD_GET_MAC_ADDR)) {
		return 0;
	}

	return 1;
}

uint8_t GSClass::send_cmd(uint8_t cmd)
{
	flush();

	memset(cmd_buffer, 16, 0);
	strcpy_P(cmd_buffer, (char*)pgm_read_word(&(cmd_tbl[cmd])));
	String cmd_str = String(cmd_buffer);

	switch(cmd) {
	case CMD_DISABLE_ECHO:
	case CMD_DISABLE_DHCP:
	case CMD_DISCONNECT:
	case CMD_ENABLE_DHCP:
	case CMD_GET_MAC_ADDR:
	case CMD_WIRELESS_MODE:
	case CMD_ENABLE_DHCPSVR:
	{
		Serial.println(cmd_str);
		break;
	}
	case CMD_SET_WPA_PSK:
	{
		String cmd_buf = cmd_str + this->security_key;
		Serial.println(cmd_buf);
		break;
	}
	case CMD_SET_SSID:
	{
		String cmd_buf;
		if (mode == 0)
			cmd_buf = cmd_str + this->ssid;
		else if (mode == 2)
			cmd_buf = cmd_str + this->ssid + ",,11";
		Serial.println(cmd_buf);
		break;
	}
	case CMD_TCP_CONN:
	case CMD_UDP_CONN:
	{
		String cmd_buf = cmd_str + this->ip + "," + this->port;
		Serial.println(cmd_buf);
		break;
	}
	case CMD_NETWORK_SET:
	{
		//String cmd_buf = cmd_tbl[cmd].cmd_str + this->local_ip + "," + this->subnet + "," + this->gateway;
		String cmd_buf = cmd_str + this->local_ip;
		cmd_buf +=  ",";
		cmd_buf += this->subnet;
		cmd_buf += ",";
		cmd_buf += this->gateway;
		Serial.println(cmd_buf);
		break;
	}
	case CMD_DNS_LOOKUP:
	{
		String cmd_buf = cmd_str + this->dns_url_ip;
		Serial.println(cmd_buf);
		break;
	}
	case CMD_CLOSE_CONN:
	{
		if (this->sock_table[socket_num].status != SOCK_STATUS::CLOSED) {
			String cmd_buf = cmd_str + String((unsigned int)this->sock_table[socket_num].cid);
			Serial.println(cmd_buf);
		} else {
			return 0;
		}
		break;
	}
	case CMD_TCP_LISTEN:
	{
		String cmd_buf = cmd_str + String((unsigned int)this->sock_table[socket_num].port);
		Serial.println(cmd_buf);
		break;
	}
	case CMD_UDP_LISTEN:
	{
	    String cmd_buf = cmd_str + String((unsigned int)this->sock_table[socket_num].port);
	    Serial.println(cmd_buf);
	    break;
	}
	default:
		break;
	}

	return 1;
}

uint8_t GSClass::parse_resp(uint8_t cmd)
{
	uint8_t resp_done = 0;
	uint8_t ret = 0;
	String buf;

	while (!resp_done) {
		buf = readline();

		switch(cmd) {
		case CMD_DISABLE_ECHO:
		case CMD_DISABLE_DHCP:
		case CMD_DISCONNECT:
		case CMD_SET_WPA_PSK:
		case CMD_SET_SSID:
		case CMD_ENABLE_DHCP:
		case CMD_NETWORK_SET:
		case CMD_WIRELESS_MODE:
		case CMD_ENABLE_DHCPSVR:
		{
			if (buf == "OK") {
				/* got OK */
				ret = 1;
				resp_done = 1;
			} else if (buf.startsWith("ERROR")) {
				/* got ERROR */
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_TCP_LISTEN:
		case CMD_UDP_LISTEN:
		{
			if (buf.startsWith("CONNECT")) {
				/* got CONNECT */
				serv_cid = hex_to_int(buf[8]);
				this->sock_table[socket_num].cid = hex_to_int(buf[8]);
				this->sock_table[socket_num].status = SOCK_STATUS::LISTEN;
				
				/* init a socket for UDP */
				if (this->sock_table[socket_num].protocol == IPPROTO::UDP) {
				    for (int new_sock = 0; new_sock < 4; new_sock++) {
				        if (this->sock_table[new_sock].status == SOCK_STATUS::CLOSED) {
				            this->sock_table[new_sock].cid = this->sock_table[socket_num].cid;
				            this->sock_table[new_sock].port = this->sock_table[socket_num].port;
				            this->sock_table[new_sock].protocol = this->sock_table[socket_num].protocol;
				            this->sock_table[new_sock].status = SOCK_STATUS::ESTABLISHED;

				            break;
				        }
				    }
				}

			} else if (buf == "OK") {
				/* got OK */
				ret = 1;
				resp_done = 1;
			} else if (buf.startsWith("ERROR")) {
				/* got ERROR */
				serv_cid = INVALID_CID;
				this->sock_table[socket_num].cid = 0;
				this->sock_table[socket_num].status = SOCK_STATUS::CLOSED;
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_TCP_CONN:
		case CMD_UDP_CONN:
		{
			if (buf.startsWith("CONNECT")) {
				/* got CONNECT */
				client_cid = hex_to_int(buf[8]);
				this->sock_table[socket_num].cid = hex_to_int(buf[8]);
				this->sock_table[socket_num].status = SOCK_STATUS::ESTABLISHED;
			} else if (buf == "OK") {
				/* got OK */
				ret = 1;
				resp_done = 1;
			} else if (buf.startsWith("ERROR")) {
				/* got ERROR */
				client_cid = INVALID_CID;
				this->sock_table[socket_num].cid = 0;
				this->sock_table[socket_num].status = SOCK_STATUS::CLOSED;
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_GET_MAC_ADDR:
		{
			if (buf.startsWith("00")) {
				/* got MAC addr */
				dev_id = buf;
			} else if (buf == "OK") {
				/* got OK */
				ret = 1;
				resp_done = 1;
			} else if (buf.startsWith("ERROR")) {
				/* got ERROR */
				dev_id = "ff:ff:ff:ff:ff:ff";
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_DNS_LOOKUP:
		{
			if (buf.startsWith("IP:")) {
				/* got IP address */
				dns_url_ip = buf.substring(3);
			} else if (buf == "OK") {
				/* got OK */
				ret = 1;
				resp_done = 1;
			} else if (buf.startsWith("ERROR")) {
				/* got ERROR */
				ret = 0;
				resp_done = 1;
			}
			break;
		}
		case CMD_CLOSE_CONN:
		{
		    if (buf == "OK") {
		        /* got OK */
		        ret = 1;
		        resp_done = 1;

		        /* clean up socket */
		        this->sock_table[socket_num].status = 0;
		        this->sock_table[socket_num].cid = 0;
		        this->sock_table[socket_num].port = 0;
		        this->sock_table[socket_num].protocol = 0;
				
				dev_mode = DEV_OP_MODE_COMMAND;
				
				/* clear flag */
				dataOnSock = 255;
		    } else if (buf.startsWith("ERROR")) {
		        /* got ERROR */
		        ret = 0;
		        resp_done = 1;
		    }
		    break;
		}
		default:
			break;
		}
	}

	return ret;
}

uint8_t GSClass::send_cmd_w_resp(uint8_t cmd)
{
	if (send_cmd(cmd)) {
		return parse_resp(cmd);
	} else {
		return 0;
	}
}

void GSClass::configure(GS_PROFILE *prof)
{
	// configure params
	this->ssid         = prof->ssid;
	this->security_key = prof->security_key;
	this->local_ip     = prof->ip;
	this->subnet       = prof->subnet;
	this->gateway      = prof->gateway;
}

uint8_t GSClass::connect()
{

	if (!send_cmd_w_resp(CMD_DISCONNECT)) {
		return 0;
	}

	if (!send_cmd_w_resp(CMD_DISABLE_DHCP)) {
		return 0;
	}

	if (mode == 0) {
	    if (this->security_key != NULL) {
		    if (!send_cmd_w_resp(CMD_SET_WPA_PSK)) {
			    return 0;
		    }
	    }

		if (!send_cmd_w_resp(CMD_SET_SSID)) {
			return 0;
		}

		if (this->local_ip == NULL) {
			if (!send_cmd_w_resp(CMD_ENABLE_DHCP)) {
				return 0;
			}
		} else {
			if (!send_cmd_w_resp(CMD_NETWORK_SET)) {
				return 0;
			}
		}

	} else if (mode == 2) {
		if (!send_cmd_w_resp(CMD_NETWORK_SET)) {
                	return 0;
                }
		if (!send_cmd_w_resp(CMD_WIRELESS_MODE)) {
                        return 0;
                }
		if (!send_cmd_w_resp(CMD_SET_SSID)) {
                        return 0;
                }
		if (!send_cmd_w_resp(CMD_ENABLE_DHCPSVR)) {
			return 0;
		}
		
	} 

	connection_state = DEV_CONN_ST_CONNECTED;

	return 1;
}

uint8_t GSClass::connected()
{
	return connection_state;
}

String GSClass::readline(void)
{
	String strBuf;
	char inByte;

	bool endDetected = false;

	while (!endDetected)
	{
		if (Serial.available())
		{
			// valid data in HW UART buffer, so check if it's \r or \n
			// if so, throw away
			// if strBuf length greater than 0, then this is a true end of line,
			// so break out
			inByte = Serial.read();

			if ((inByte == '\r') || (inByte == '\n'))
			{
				// throw away
				if ((strBuf.length() > 0) && (inByte == '\n'))
				{
					endDetected = true;
				}
			}
			else
			{
				strBuf += inByte;
			}
		}
	}

	return strBuf;
}

uint16_t GSClass::readData(SOCKET s, uint8_t* buf, uint16_t len)
{
    uint16_t dataLen = 0;
    uint8_t tmp1, tmp2;

    if (dev_mode == DEV_OP_MODE_DATA_RX) {
        if (!Serial.available())
            return 0;

        while(dataLen < len) {
            if (Serial.available()) {
                tmp1 = Serial.read();

                if (tmp1 == 0x1b) {
                    // escape seq

                    /* read in escape sequence */
                    while(1) {
                        if (Serial.available()) {
                            tmp2 = Serial.read();
                            break;
                        }
                    }

                    if (tmp2 == 0x45) {
                        /* data end, switch to command mode */
                        dev_mode = DEV_OP_MODE_COMMAND;
                        /* clear flag */
                        dataOnSock = 255;
                        break;
                    } else {
                        if (dataLen < (len-2)) {
                            buf[dataLen++] = tmp1;
                            buf[dataLen++] = tmp2;
                        } else {
                            buf[dataLen++] = tmp1;

                            /* FIXME : throw away second byte ? */
                        }
                    }
                } else {
                    // data
                    buf[dataLen] = tmp1;
                    dataLen++;
                }
            }
        }
    }

    return dataLen;
}

uint16_t GSClass::writeData(SOCKET s, const uint8_t*  buf, uint16_t  len)
{	
	if ((len == 0) || (buf[0] == '\r')){
	} else {
	    if ((this->sock_table[s].protocol == IPPROTO::TCP) ||
	            (this->sock_table[s].protocol == IPPROTO::UDP_CLIENT)) {
	        Serial.write((uint8_t)0x1b);    // data start
	        Serial.write((uint8_t)0x53);
	        Serial.write((uint8_t)int_to_hex(this->client_cid));  // connection ID
	        if (len == 1){
	            if (buf[0] != '\r' && buf[0] != '\n'){
	                Serial.write(buf[0]);           // data to send
	            } else if (buf[0] == '\n') {
	                Serial.print("\n\r");           // new line
	            }
	        } else {
	            String buffer;
	            buffer = (const char *)buf;
	            Serial.print(buffer);
	        }
	        Serial.write((uint8_t)0x1b);    // data end
	        Serial.write((uint8_t)0x45);
	    } else if (this->sock_table[s].protocol == IPPROTO::UDP) {
	        Serial.write((uint8_t)0x1b);    // data start
	        Serial.write((uint8_t)0x55);
	        Serial.write((uint8_t)int_to_hex(this->sock_table[s].cid));  // connection ID
	        Serial.print(srcIPUDP);
	        Serial.print(":");
	        Serial.print(srcPortUDP);
	        Serial.print(":");
	        if (len == 1){
	            if (buf[0] != '\r' && buf[0] != '\n'){
	                Serial.write(buf[0]);           // data to send
	            } else if (buf[0] == '\n') {
	                Serial.print("\n\r");           // new line
	            }
	        } else {
	            String buffer;
	            buffer = (const char *)buf;
	            Serial.print(buffer);
	        }
	        Serial.write((uint8_t)0x1b);    // data end
	        Serial.write((uint8_t)0x45);
	    }
	}
	delay(10);

    return 1;
}

void GSClass::process()
{
    String strBuf;
    char inByte;
    uint8_t processDone = 0;

    if (!Serial.available())
        return;

    while (!processDone) {
        if (dev_mode == DEV_OP_MODE_COMMAND) {
            while (1) {
                if (Serial.available()) {
                    inByte = Serial.read();

                    if (inByte == 0x1b) {
                        // escape seq

                        // switch mode
                        dev_mode = DEV_OP_MODE_DATA;
                        break;
                    } else {
                        // command string
                        if ((inByte == '\r') || (inByte == '\n')) {
                            // throw away
                            if ((strBuf.length() > 0) && (inByte == '\n'))
                            {
                                // parse command
                                parse_cmd(strBuf);
                                processDone = 1;
                                break;
                            }
                        }
                        else
                        {
                            strBuf += inByte;
                        }
                    }
                }
            }
        } else if (dev_mode == DEV_OP_MODE_DATA) {
            /* data mode */
            while(1) {
				//digitalWrite(5, LOW);
                if (Serial.available()) {
                    inByte = Serial.read();

                    if (inByte == 0x53) {
                        /* TCP data start, switch to data RX mode */
                        dev_mode = DEV_OP_MODE_DATA_RX;
                        /* read in CID */
                        while(1) {
                            if (Serial.available()) {
                                inByte = Serial.read();
								
                                break;
                            }
                        }

                        // find socket from CID
                        for (SOCKET new_sock = 0; new_sock < 4; new_sock++) {
                            if (this->sock_table[new_sock].cid == hex_to_int(inByte)) {
                                dataOnSock = new_sock;
								break;
                            }
                        }

                        break;
                    } else if (inByte == 0x75) {
                        /* UDP data start, switch to data RX mode */
                        dev_mode = DEV_OP_MODE_DATA_RX;
                        /* read in CID */
                        while(1) {
                            if (Serial.available()) {
                                inByte = Serial.read();
                                break;
                            }
                        }

                        // find socket from CID
                        for (SOCKET new_sock = 0; new_sock < 4; new_sock++) {
                            if (this->sock_table[new_sock].cid == hex_to_int(inByte)) {
                                if (this->sock_table[new_sock].status == SOCK_STATUS::ESTABLISHED) {
                                    dataOnSock = new_sock;
                                    break;
                                }
                            }
                        }

                        /* read in source IP address */
                        srcIPUDP = "";
                        while(1) {
                            if (Serial.available()) {
                                inByte = Serial.read();

                                if (inByte == 0x20) {
                                    /* space */
                                    break;
                                } else {
                                    srcIPUDP += inByte;
                                }
                            }
                        }

                        /* read in source port number */
                        srcPortUDP = "";
                        while(1) {
                            if (Serial.available()) {
                                inByte = Serial.read();

                                if (inByte == 0x09) {
                                    /* horizontal tab */
                                    break;
                                } else {
                                    srcPortUDP += inByte;
                                }
                            }
                        }

                        break;
                    } else if (inByte == 0x45) {
                        /* data end, switch to command mode */
                        dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    } else if (inByte == 0x4f) {
                        /* data mode ok */
                        tx_done = 1;
                        dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    } else if (inByte == 0x46) {
                        /* TX failed */
                        tx_done = 1;
                        dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    } else {
                        /* unknown */
                        dev_mode = DEV_OP_MODE_COMMAND;
                        processDone = 1;
                        break;
                    }
                }
            }
        } else if (dev_mode ==  DEV_OP_MODE_DATA_RX) {
			//digitalWrite(6, LOW);
            processDone = 1;
        }
    }
}

void GSClass::parse_cmd(String buf)
{
	if (buf.startsWith("CONNECT")) {
		/* got CONNECT */

		for (int sock = 0; sock < 4; sock++) {
			if ((this->sock_table[sock].status == SOCK_STATUS::LISTEN) &&
				(this->sock_table[sock].cid == hex_to_int(buf[8])))
			{
			    if (this->sock_table[sock].protocol == IPPROTO::TCP) {
			        if (serv_cid == hex_to_int(buf[8])) {
			            /* client connected */
			            client_cid = hex_to_int(buf[10]);
			        }

			        for (int new_sock = 0; new_sock < 4; new_sock++) {
			            if (this->sock_table[new_sock].status == SOCK_STATUS::CLOSED) {
			                this->sock_table[new_sock].cid = hex_to_int(buf[10]);
			                this->sock_table[new_sock].port = this->sock_table[sock].port;
			                this->sock_table[new_sock].protocol = this->sock_table[sock].protocol;
			                this->sock_table[new_sock].status = SOCK_STATUS::ESTABLISHED;
			                break;
			            }
			        }
			    }
			}
		}

	} else if (buf.startsWith("DISCONNECT")) {
		/* got disconnect */
		digitalWrite(6, LOW);
		for (int sock = 0; sock < 4; sock++) {
			if ((this->sock_table[sock].status == SOCK_STATUS::ESTABLISHED) &&
				(this->sock_table[sock].cid == hex_to_int(buf[11])))
			{
				this->sock_table[sock].cid = 0;
				this->sock_table[sock].port = 0;
				this->sock_table[sock].protocol = 0;
				this->sock_table[sock].status = SOCK_STATUS::CLOSED;
				break;
			}
		}
		// FIXME : need to handle socket disconnection
	} else if (buf == "Disassociation Event") {
		/* disconnected from AP */
		connection_state = DEV_CONN_ST_DISCONNECTED;
	}
}

void GSClass::parse_data(String buf)
{
	this->rx_data_handler(buf);
}

uint8_t GSClass::connectSocket(SOCKET s, String ip, String port)
{
    uint8_t cmd = CMD_INVALID;

	this->ip = ip;
	this->port = port;
	this->socket_num = s;

	if (this->sock_table[s].protocol == IPPROTO::TCP) {
	    cmd = CMD_TCP_CONN;
	} else if (this->sock_table[s].protocol == IPPROTO::UDP_CLIENT) {
	    cmd = CMD_UDP_CONN;
	}

	if (!send_cmd_w_resp(cmd)) {
		return 0;
	}

	return 1;
}

String GSClass::dns_lookup(String url)
{
	this->dns_url_ip = url;

	if (!send_cmd_w_resp(CMD_DNS_LOOKUP)) {
		return String("0.0.0.0");
	}

	return this->dns_url_ip;
}

String GSClass::get_dev_id()
{
	return dev_id;
}

void GSClass::configSocket(SOCKET s, uint8_t protocol, uint16_t port)
{
	this->sock_table[s].protocol = protocol;
	this->sock_table[s].port = port;
	this->sock_table[s].status = SOCK_STATUS::INIT;
}

void GSClass::execSocketCmd(SOCKET s, uint8_t cmd)
{
	this->socket_num = s;

	if (!send_cmd_w_resp(cmd)) {
	}
}

uint8_t GSClass::readSocketStatus(SOCKET s)
{
	return this->sock_table[s].status;
}

uint8_t GSClass::getSocketProtocol(SOCKET s)
{
	return this->sock_table[s].protocol;
}

uint8_t GSClass::isDataOnSock(SOCKET s)
{
    return (s == dataOnSock);
}

void GSClass::flush()
{
	// arduino-1.0 repurposed the Serial.flush() command
	// to wait for outgoing data to be transmitted, not to
	// clear the buffer
	// since we need to clear the buffer, need to create this
	// workaround
	while (Serial.available())
	{
		Serial.read();
	}
}

