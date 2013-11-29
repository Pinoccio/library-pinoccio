#include <Arduino.h>
#include <backpacks/wifi/WiFiBackpack.h>

WiFiBackpack::WiFiBackpack() { }

WiFiBackpack::~WiFiBackpack() { }

void WiFiBackpack::setup() { }

void WiFiBackpack::loop() {

  // TODO if AP or HQ connection is gone, reconnect here
}

void WiFiBackpack::wifiConfig(const char *ssid, uint8_t ssidSize, const char *passphrase, uint8_t passSize) {
  // TODO
  profile = {
          /* SSID */ "",
          /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
          /* subnet mask */ "",
          /* Gateway IP */ "" };
}

void WiFiBackpack::printAccessPoints() {
  // TODO
  // addBitlashFunction("wifi.listaps", (bitlash_function) isBatteryCharging);
}

bool WiFiBackpack::connectToAP() {
  Wifi.begin(&profile);
}

bool WiFiBackpack::connectToHQ() {
  // TODO
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