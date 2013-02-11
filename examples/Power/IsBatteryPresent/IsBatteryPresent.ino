#include <Pinoccio.h>

void setup() {
  Pinoccio.init();
  
  if (Pinoccio.isBatteryPresent()) {
    Serial.println("A battery is present.");
  } else {
    Serial.println("No battery is present.");
  }
}

void loop() {
  Pinoccio.loop();
}
