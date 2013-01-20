#include <Pinoccio.h>

WIFI_PROFILE profile = {
        /* SSID */ "",
        /* WPA/WPA2 passphrase */ "",
        /* IP address */ "",
        /* subnet mask */ "",
        /* Gateway IP */ "", };

  IPAddress server(66,175,218,211);

  void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Received MQTT packet: ");
    Serial.write(payload, length);
    Serial.println("");
  }

  PinoccioWifiClient wifiClient;
  mqttClient mqtt(server, 1883, callback, wifiClient);

  void setup() {
    Pinoccio.init();
    
    Serial.println("Starting wireless...");

    Wifi.begin(&profile);
    Serial.println("Done");
    
    Serial.println("Connecting to MQTT server...");
    if (mqtt.connect("pinoccio", "erictj", "321")) {
      Serial.println("Done");
      Serial.println("Publishing first MQTT packet...");
      mqtt.publish("clienttest","hello world");
      Serial.println("Done");
      Serial.println("Subscribing to colorpicker");
      mqtt.subscribe("colorpicker");
      Serial.println("Done");
    }
    Serial.println("Done with MQTT server connection attempt");
  }

  void loop() {
    mqtt.loop();
  }

