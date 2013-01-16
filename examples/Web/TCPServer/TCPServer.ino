#include <Wirefree.h>
#include <WifiServer.h>

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ "192.168.1.1",
                 /* subnet mask */ "255.255.255.0",
                  /* Gateway IP */ "192.168.1.1", };

WifiServer server(12345, PROTO_TCP);

void setup()
{
  // connect to AP & start server
  Wireless.begin(&wireless_prof, AP_MODE);
  server.begin();
  
  delay(1000);
}

void loop()
{
  // Listen for incoming clients
  WifiClient client = server.available();

  if (client) {
    // if there are incoming bytes available
    // from the peer device, read them and print them:
    while (client.available()) {
      int in;

      while ((in = client.read()) == -1);

      Serial.print((char)in);

      if (in == 0xa)
          break;
    }

    // Send message over UDP socket to peer device
    client.println("Hello client!");

    delay(1);
  }
}
