#include "config.h"
#include <Pinoccio.h>

static char temp[6];

void setup() {
  Pinoccio.setup();
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