#include <Arduino.h>
#include <Scout.h>

PinoccioScout Scout;

PinoccioScout::PinoccioScout() {
  RgbLed.turnOff();
  pinMode(CHG_STATUS, INPUT);
  pinMode(BATT_ALERT, INPUT);
  digitalWrite(BATT_ALERT, HIGH);
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  pinMode(BACKPACK_BUS, INPUT);
  
  leadScoutAddress = 0;
  backpacks[0] = NULL;
}

PinoccioScout::~PinoccioScout() { }

void PinoccioScout::setup() { 
  Pinoccio::setup();
}

void PinoccioScout::loop() { 
  Pinoccio::loop();
}

bool PinoccioScout::isBatteryCharging() {
  return (digitalRead(CHG_STATUS) == LOW);
}

int PinoccioScout::getBatteryPercentage() {
  return 0;
}

void PinoccioScout::enableBackpackVcc() {
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioScout::disableBackpackVcc() {
  digitalWrite(VCC_ENABLE, LOW);
}