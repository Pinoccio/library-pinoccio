/*
Wirefree.cpp - interface class to talk with DIYSandbox Arduino devices 

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

#include "Wirefree.h"
#include "gs.h"

uint16_t Wirefree::_server_port[MAX_SOCK_NUM] = {
  0, 0, 0, 0 };

void Wirefree::initLED()
{
	unsigned char i;
	
	pinMode(6, OUTPUT);         // RED
	pinMode(5, OUTPUT);         // GREEN
	pinMode(3, OUTPUT);         // BLUE
	digitalWrite(6, HIGH);
	digitalWrite(5, HIGH);
	digitalWrite(3, HIGH);
	
	for (i = 0; i < 3; i++) {
		digitalWrite(((i==0)?6:((i==1)?5:((i==2)?3:6))), LOW);
		delay(1000);
		digitalWrite(6, HIGH);
		digitalWrite(5, HIGH);
		digitalWrite(3, HIGH);
	}

}

void Wirefree::setLED(int color)
{
	// Clear LED
	digitalWrite(3, HIGH);
	digitalWrite(5, HIGH);
	digitalWrite(6, HIGH);
	
	if (color == LED_BLUE){
		digitalWrite(3, LOW);   
	} else if (color == LED_GREEN){
		digitalWrite(5, LOW);   
	} else if (color == LED_RED){
		digitalWrite(6, LOW);
        } else if (color == LED_CYAN){
        	digitalWrite(3, LOW);
        	digitalWrite(5, LOW);
        } else if (color == LED_MAGENTA){
        	digitalWrite(6, LOW);
        	digitalWrite(3, LOW);       
	} else if (color == LED_YELLOW){
        	digitalWrite(5, LOW);
        	digitalWrite(6, LOW);
       	} else if (color == LED_WHITE){
        	digitalWrite(6, LOW);
        	digitalWrite(5, LOW);
        	digitalWrite(3, LOW);           
	} else {
	}	
}

void Wirefree::begin(WIFI_PROFILE* w_prof)
{
	begin(w_prof, NORMAL_MODE);
}

void Wirefree::begin(WIFI_PROFILE* w_prof, uint8_t mode)
{
	// setup LEDs
	initLED();

	GS.mode = mode;
	
	// initialize device
	if (!GS.init()) {
		setLED(LED_RED);
		return;
	}

	// configure params
	GS.configure((GS_PROFILE*)w_prof);

	// initiate wireless connection
	while (!GS.connect());
	
	setLED(LED_GREEN);
}

void Wirefree::process()
{
	GS.process();
}

uint8_t Wirefree::socketOpen(String url, String port)
{
#if 0
	String ip;

	// get IP address from URL
	if ((ip = GS.dns_lookup(url)) == "0.0.0.0") {
		return 0;
	}

	// open socket connection
	if (!GS.connect_socket("192.168.0.100", "32000")) {
		return 0;
	}
#endif
	return 1;
}

Wirefree Wireless;

