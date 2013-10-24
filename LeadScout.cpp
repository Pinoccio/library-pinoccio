#include <Arduino.h>
#include <LeadScout.h>

WIFI_PROFILE profile = {
                /* SSID */ "",
 /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };

PinoccioLeadScout LeadScout;
FlashClass Flash(SS, SPI);

PinoccioLeadScout::PinoccioLeadScout() {
  scouts[0] = NULL;
  //TODO 
  //IPAddress server(8,8,8,8);

  //mqttClient mqtt(server, 1883, mqttMessageReceived, netClient);

  //Wifi.begin(&profile);

  //mqtt.connect("pinoccio");
}

PinoccioLeadScout::~PinoccioLeadScout() { }


void PinoccioLeadScout::setup() {
  PinoccioScout::setup();
}

void PinoccioLeadScout::loop() {
  PinoccioScout::loop();
  //mqtt.loop();
}

static void mqttMessageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT packet from topic : ");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");
  Serial.println("Sending message over mesh network");
  //meshSendMessage(payload, length);
}
