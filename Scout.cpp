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
  
  digitalWrite(SS, HIGH);
  pinMode(SS, OUTPUT);
  
  pinMode(BACKPACK_BUS, INPUT);
  
  leadScoutAddresses[0] = NULL;
  backpacks[0] = NULL;
}

PinoccioScout::~PinoccioScout() { }


void PinoccioScout::setup() { 
  PinoccioClass::setup();
  
  Wire.begin(); 
  delay(100);
  HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%
  HAL_FuelGaugeQuickStart(); // Restart fuel-gauge calculations
}

void PinoccioScout::loop() { 
  PinoccioClass::loop();
}

bool PinoccioScout::isBatteryCharging() {
  return (digitalRead(CHG_STATUS) == LOW);
}

int PinoccioScout::getBatteryPercentage() {
  return constrain(HAL_FuelGaugePercent(), 0, 100);
}

float PinoccioScout::getBatteryVoltage() {
  return HAL_FuelGaugeVoltage();
}

void PinoccioScout::enableBackpackVcc() {
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioScout::disableBackpackVcc() {
  digitalWrite(VCC_ENABLE, LOW);
}