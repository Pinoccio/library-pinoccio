#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();
  
  Serial.print("Battery percentage: ");
  Serial.println(Scout.getBatteryPercentage());
  
  Serial.print("Battery charging:   ");
  if (Scout.isBatteryCharging()) {
    Serial.println("yes");
  } else {
    Serial.println("no");
  }
  
  Serial.println("");
  delay(3000);
}