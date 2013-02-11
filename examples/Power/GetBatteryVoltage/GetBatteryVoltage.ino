#include <Pinoccio.h>

int voltage;

void setup() {
  Pinoccio.init();
}

void loop() {
  Pinoccio.loop();
  
  voltage = Pinoccio.getBatteryVoltage();
  Serial.print("Battery voltage: ");
  Serial.println(voltage);
  delay(3000);
}