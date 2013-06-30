#include <Arduino.h>
#include <Wire.h>

#include "Scout.h"
#include <math.h>

PinoccioScout Scout;

PinoccioScout::PinoccioScout() {
  RgbLed.turnOff();
  
  digitalWrite(CHG_STATUS, HIGH);
  pinMode(CHG_STATUS, INPUT);
  
  digitalWrite(BATT_ALERT, HIGH);
  pinMode(BATT_ALERT, INPUT);
  
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  
  pinMode(BACKPACK_BUS, INPUT);
  
  leadScoutAddress = 0;
  backpacks[0] = NULL;
}

PinoccioScout::~PinoccioScout() { }

void PinoccioScout::setup() { 
  PinoccioClass::setup();
  
  Wire.begin(); 
  delay(100);
  HAL_configMAX17048G(20);   // Configure the MAX17048G's alert percentage
  HAL_qsMAX17048G();         // restart fuel-gauge calculations
}

void PinoccioScout::loop() { 
  PinoccioClass::loop();
}

bool PinoccioScout::isBatteryCharging() {
  return (digitalRead(CHG_STATUS) == LOW);
}

float PinoccioScout::getBatteryPercentage() {
  return roundf(HAL_percentMAX17048G() * 10.0f) / 10.0f;
}

float PinoccioScout::getBatteryVoltage() {
  return roundf((HAL_vcellMAX17048G() * 1/800) * 10.0f) / 10.0f;
}

void PinoccioScout::enableBackpackVcc() {
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioScout::disableBackpackVcc() {
  digitalWrite(VCC_ENABLE, LOW);
}