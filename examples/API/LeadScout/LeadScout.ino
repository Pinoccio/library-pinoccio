#include <Pinoccio.h>

WIFI_PROFILE profile = {
                /* SSID */ "",
 /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };

IPAddress server(66,175,218,211);
//IPAddress server(85,119,83,194);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT packet from topic : ");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");
}

PinoccioWifiClient wifiClient;
mqttClient mqtt(server, 1883, callback, wifiClient);

void setup() {
  Pinoccio.init();
  Serial.print("Starting wireless...");
  Wifi.begin(&profile);
  Serial.println("Done");
  
  Serial.println("Connecting to MQTT server...");
  if (mqtt.connect("pinoccio", "username", "password")) {
    Serial.println("Done");
    Serial.println("Publishing first MQTT packet...");
    mqtt.publish("erictj/from-pinoccio", "hello world");
    Serial.println("Done");
    Serial.println("Subscribing to topic");
    mqtt.subscribe("erictj/to-pinoccio");
    Serial.println("Done");
  }
  Serial.println("Done with MQTT server connection attempt");
}

void loop() {
  //Pinoccio.loop();
  mqtt.loop();
}