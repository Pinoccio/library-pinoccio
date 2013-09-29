/*
WebClient.pde - Web Client Arduino processing sketch

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

#include <Wirefree.h>
#include <WifiClient.h>

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ NULL,
                 /* subnet mask */ NULL,
                  /* Gateway IP */ NULL, };

String server = "74.125.224.83"; // Google

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WifiClient client(server, "80", PROTO_TCP);

void setup()
{
  // connect to AP & start server
  Wireless.begin(&wireless_prof);
  
  // if you get a connection, report back via serial:
  if (client.connect()) {
    Serial.println("connection Success..");
    
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.0\r\n\r\n");
    client.flush();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed..");
  }
    
//  delay(1000);
}

void loop()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
      // Uncomment if you need to see the response in the serial monitor
 //   Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println("disconnecting.");
    client.stop();

    // do nothing forevermore:
    for(;;)
      ;
  }  

}
