#include <Pinoccio.h>

void setup() {
  Pinoccio.init();
}

void loop() {
  Pinoccio.loop();
  
  if (Pinoccio.isBatteryCharging()) {
    Serial.println("Battery is charging.");
  } else {
    Serial.println("Battery is not charging.");
  }
  delay(3000);
}