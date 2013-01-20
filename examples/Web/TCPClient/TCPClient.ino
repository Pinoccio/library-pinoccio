#include <Pinoccio.h>

WIFI_PROFILE profile = {
        /* SSID */ "",
        /* WPA/WPA2 passphrase */ "",
        /* IP address */ "",
        /* subnet mask */ "",
        /* Gateway IP */ "" };

IPAddress server(192,168,1,1);
int port = 12345;

PinoccioWifiClient client;

void setup() {
  Pinoccio.init();

  Wifi.begin(&profile);

  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected");

    // Send message over UDP socket to peer device
    client.println("Hello server!");
  }
  else {
    // if connection setup failed:
    Serial.println("failed");
  }
}

void loop()
{
  // if there are incoming bytes available
  // from the peer device, read them and print them:
  while (client.available()) {
    int in;

    while ((in = client.read()) == -1);

    Serial.print((char)in);
  }

  delay(1);
}
