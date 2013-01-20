#include <Pinoccio.h>

WIFI_PROFILE profile = {
                        /* SSID */ "",
         /* WPA/WPA2 passphrase */ "",
                  /* IP address */ "",
                 /* subnet mask */ "",
                  /* Gateway IP */ "", };

IPAddress server(74,125,224,83); // peer device IP address
int port = 80;

PinoccioWifiClient client();

void setup() {
  Pinoccio.init();
  
  Wifi.begin(&profile);
  
  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
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
