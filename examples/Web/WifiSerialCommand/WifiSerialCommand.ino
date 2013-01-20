#include <Pinoccio.h>

WIFI_PROFILE profile = {
        /* SSID */ "",
        /* WPA/WPA2 passphrase */ "",
        /* IP address */ "",
        /* subnet mask */ "",
        /* Gateway IP */ "", };

void setup() {
  Pinoccio.init();
  Serial.println("Starting up");
  Serial.println("Starting wireless...");

  Wifi.begin(&profile);

  Serial.println("Done");
  delay(1000);
}

void loop() {
  Pinoccio.taskHandler();

  // read from port 1, send to port 0:
  while (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte); 
  }

  // read from port 0, send to port 1:
  while (Serial.available()) {
    int inByte = Serial.read();
    Serial1.write(inByte); 
  }
}
