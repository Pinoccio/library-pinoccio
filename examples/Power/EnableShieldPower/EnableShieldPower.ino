#include <Pinoccio.h>

void setup() {
  Pinoccio.init();
}

void loop() {
  Pinoccio.loop();
  
  Pinoccio.enableShieldVcc();
  Serial.println("Shield VCC is on.");
  delay(3000);
  
  Pinoccio.disableShieldVcc();
  Serial.println("Shield VCC is off.");
  delay(3000);
}