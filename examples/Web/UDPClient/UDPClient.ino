#include <Wirefree.h>
#include <WifiClient.h>

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ NULL,
                  /* IP address */ "192.168.1.2",
                 /* subnet mask */ "255.255.255.0",
                  /* Gateway IP */ "192.168.1.1", };

String remote_server = "192.168.1.1"; // peer device IP address
String remote_port = "12345";

// Initialize client with IP address and port number
WifiClient client(remote_server, remote_port, PROTO_UDP);

void setup()
{
  // connect to AP
  Wireless.begin(&wireless_prof);
  
  // if you get a connection, report back via serial:
  if (client.connect()) {
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
