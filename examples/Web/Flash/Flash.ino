#include <LeadScout.h>

void setup() {   
  LeadScout.setup();
  Serial.begin(115200);
  Serial.print("Starting Flash Test: "); 
  
  while (!Flash.available()) { }
  
  Serial.println("chip ready");
}

void loop() {
  LeadScout.loop();
  char data_to_chip[128] = "Testing 9";
  char data_from_chip[128] = "         ";
  int i = 0;
  
  Serial.print("Writing to flash: ");
  Serial.println(data_to_chip);

  // Write some data to RAM
  Flash.write(0, data_to_chip, 9);
  delay(100);

  Serial.print("Reading from flash: ");
  // Read it back to a different buffer
  Flash.read(0, data_from_chip, 9);
 
  // Write it to the serial port
  for (i = 0; i < 10; i++) {
    Serial.write(data_from_chip[i]);
  }
  Serial.print("\n");
  delay(5000);                  // wait for a second
}