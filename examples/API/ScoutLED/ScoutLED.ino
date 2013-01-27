#include <Pinoccio.h>

WIFI_PROFILE profile = {
                /* SSID */ "",
 /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };

IPAddress server(66,175,218,211);
//IPAddress server(85,119,83,194);

PinoccioWifiClient wifiClient;
mqttClient mqtt(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT packet from topic : ");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");

  RgbLed.setHex((char*)payload);
}

void setup() {
  Pinoccio.init();
  Wifi.begin(&profile);

  if (mqtt.connect("pinoccio")) {
    mqtt.subscribe("erictj/colorwheel");
  }
}

void loop() {
  Pinoccio.loop();
  mqtt.loop();
}
