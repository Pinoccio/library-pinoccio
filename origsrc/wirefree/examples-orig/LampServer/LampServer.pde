/*
Wireless Sensor Lamp

This sketch is used in conjunction with the PingClient demo to control
a LED light strip wirelessly.  The PingServer listens for an incoming
socket connection.  Once the client has opened a socket to this server,
the server with then read the data.  If it is the value '1', then we
will toggle the light.

More details of the PingServer build-up are available on the DIYSandbox
website.

The PingServer and PingClient both need to be connected to the same
access point.

The circuit:
     * LED strip connected to pin 7
     
created Jan 21 2012
by c0demonkey

This example code is in the public domain.

*/

#include <Wirefree.h>
#include <WifiServer.h>

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ "192.168.1.7",
                 /* subnet mask */ "255.255.255.0",
                  /* Gateway IP */ "192.168.1.1", };

// port 80 is default for HTTP
WifiServer server(80, PROTO_TCP);

void setup()
{
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  delay(500);
  digitalWrite(7, LOW);
  // connect to AP & start server
  Wireless.begin(&wireless_prof);
  server.begin();

  delay(1000);
}

void loop()
{
  int val;
  // Listen for incoming clients
  WifiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c; 
        int  b; 
        
        while((b = client.read()) == -1);
        c = b;
        
        if (c == '1') {
          val = digitalRead(7);
          val = (val == 1) ? 0 : 1;
          if (val)
            digitalWrite(7, HIGH);
          else
            digitalWrite(7, LOW);
          client.flush();
          break;
        } 
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection         
    client.stop(); 
    // start a new server session
    server.begin();    
  }
}


