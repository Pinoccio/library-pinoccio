/*
Wireless Messaging Demo

This sketch will send a message to a listening server.  You can use any server
that is listening on port 3490.  It is a very simple messaging service.

You will need a server that is listening on port 3490 and can accept the incoming
data.  An example one for Linux based systems is included.

created Jan 21 2012
by c0demonkey and diysandbox

This example code is in the public domain.

*/

#include <Wirefree.h>
#include <WifiClient.h>

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ NULL,
                 /* subnet mask */ NULL,
                  /* Gateway IP */ NULL, };

String server = "192.168.1.139";

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WifiClient client(server, "3490", PROTO_TCP);

void setup()
{
  // connect to AP & start server
  Wireless.begin(&wireless_prof);
  
  // if you get a connection, report back via serial:
  if (client.connect()) {
    Serial.println("connection Success..");
    
    // Make a HTTP request:
    client.println("Hello World from DIY Sandbox\n");
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
      delay(1000);
  }  

}
