#include <Pinoccio.h>

void setup() {
  Pinoccio.init();
  
  if (Pinoccio.isBatteryCharging()) {
    Serial.println("Battery is charging.");
  } else {
    Serial.println("Battery is not charging.");
  }
}

void loop() {
  Pinoccio.loop();
}