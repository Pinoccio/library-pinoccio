#include <Scout.h>

void setup() {
  Scout.setup();
}

void loop() {
  Scout.loop();
  
  Scout.enableBackpackVcc();
  Serial.println("Backpack VCC is on.");
  delay(3000);
  
  Scout.disableBackpackVcc();
  Serial.println("Backpack VCC is off.");
  delay(3000);
}