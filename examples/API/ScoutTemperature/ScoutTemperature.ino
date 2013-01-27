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
mqttClient mqtt(server, 1883, mqttReceive, wifiClient);
static char temp[6];

void setup() {
  Pinoccio.init();
  Wifi.begin(&profile);

  mqtt.connect("pinoccio");
}

void loop() {
  Pinoccio.loop();
  mqtt.loop();
 
  if (mqtt.connected()) {
    dtostrf(Pinoccio.getTemperature(), 5, 2, temp);
    mqtt.publish("erictj/temperature", temp);
    delay(1000);
  }
}

void mqttReceive(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT packet from topic : ");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");
}