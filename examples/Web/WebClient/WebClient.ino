#include <Pinoccio.h>

WIFI_PROFILE profile = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ NULL,
                 /* subnet mask */ NULL,
                  /* Gateway IP */ NULL, };

String server = "74.125.224.83"; // Google

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
PinoccioWifiClient client(server, "80", PROTO_TCP);

void setup()
{
  // connect to AP & start server
  Wifi.begin(&profile);
  
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
    Serial.write(c);
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
