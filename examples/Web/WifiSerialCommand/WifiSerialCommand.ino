void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  // uncomment these if you want to put the WiFi module into firmware update mode
  //pinMode(6, OUTPUT);
  //digitalWrite(6, HIGH);

  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, LOW);
  delay(1000);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
}

void loop() {

  // read from port 1, send to port 0:
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
}