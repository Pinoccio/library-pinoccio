#include <Arduino.h>
#include <LeadScout.h>

WIFI_PROFILE profile = {
                /* SSID */ "Radio Free Epnk",
 /* WPA/WPA2 passphrase */ "tenor78!heavyweight",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };

PinoccioLeadScout LeadScout;

PinoccioLeadScout::PinoccioLeadScout() {
  scouts[0] = NULL;
  //TODO 
  IPAddress server(85,119,83,194);

  mqttClient mqtt(server, 1883, mqttMessageReceived, netClient);

  Wifi.begin(&profile);

  mqtt.connect("pinoccio");
}

PinoccioLeadScout::~PinoccioLeadScout() { }

void PinoccioLeadScout::setup() {
  PinoccioScout::setup();
}

void PinoccioLeadScout::loop() {
  PinoccioScout::loop();
  mqtt.loop();
}

static void mqttMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT packet from topic : ");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");
  Serial.println("Sending message over mesh network");
  //meshSendMessage(payload, length);
}