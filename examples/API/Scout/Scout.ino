#include <Pinoccio.h>

WIFI_PROFILE profile = {
        /* SSID */ "",
        /* WPA/WPA2 passphrase */ "",
        /* IP address */ "",
        /* subnet mask */ "",
        /* Gateway IP */ "", };

String server = "85.119.83.194";

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Received MQTT packet: ");
	Serial.write(payload, length);
	Serial.println("");
}

mqttClient mqtt;

void setup() {
	PinoccioWifiClient wifiClient(server, "1883", PROTO_TCP);
	mqtt = mqttClient(wifiClient, callback);
	Serial.println("Starting up");
  Pinoccio.init();
  Serial.begin(115200);
  Serial.println("Starting wireless...");

	Wifi.begin(&profile);
  delay(1000);
	Serial.println("Done");
	
	Serial.println("Connecting to MQTT server...");
	if (mqtt.connect("client")) {
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

