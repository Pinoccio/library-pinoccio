#include <Arduino.h>
#include <Wire.h>
#include "Scout.h"
#include <math.h>
#include <avr/eeprom.h>

PinoccioScout Scout;

PinoccioScout::PinoccioScout() {
  RgbLed.turnOff();

  digitalWrite(CHG_STATUS, HIGH);
  pinMode(CHG_STATUS, INPUT);

  digitalWrite(BATT_ALERT, HIGH);
  pinMode(BATT_ALERT, INPUT);

  pinMode(VCC_ENABLE, OUTPUT);
  enableBackpackVcc();

  digitalWrite(SS, HIGH);
  pinMode(SS, OUTPUT);

  pinMode(BACKPACK_BUS, INPUT);

  leadScoutAddresses[0] = NULL;
  backpacks[0] = NULL;
}

PinoccioScout::~PinoccioScout() { }


void PinoccioScout::setup() {
  enableBackpackVcc();
  PinoccioClass::setup();
  bp.begin(BACKPACK_BUS);
  Wire.begin();
  delay(100);
  HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%
  HAL_FuelGaugeQuickStart(); // Restart fuel-gauge calculations
  stateSaved = false;

  // Enumerate backpack bus
  if (!bp.enumerate()) {
    bp.printLastError(Serial);
    Serial.println();
  }

  // TODO - Find lead scouts in this network via ping and save locally
  // TODO - Enumerate attached backpacks, if any are attached
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
  vccEnabled = true;
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioScout::disableBackpackVcc() {
  vccEnabled = false;
  digitalWrite(VCC_ENABLE, LOW);
}

bool PinoccioScout::isBackpackVccEnabled() {
  return vccEnabled;
}

bool PinoccioScout::isLeadScout() {
  // Check for attached wifi backpack (model id 0x0001)
  for (uint8_t i = 0; i < bp.num_slaves; ++i) {
    if (bp.slave_ids[i][1] == 0 &&
        bp.slave_ids[i][2] == 1)
      return true;
  }
  return false;
}

void PinoccioScout::checkStateChange() {
  if (!stateSaved) {
    stateSaved = true;
    digitalPinState[0] = digitalRead(2);
    digitalPinState[1] = digitalRead(3);
    digitalPinState[2] = digitalRead(4);
    digitalPinState[3] = digitalRead(5);
    digitalPinState[4] = digitalRead(6);
    digitalPinState[5] = digitalRead(7);
    digitalPinState[6] = digitalRead(8);
    digitalPinState[7] = digitalRead(17);
    digitalPinState[8] = digitalRead(18);
    digitalPinState[9] = digitalRead(20);
    digitalPinState[10] = digitalRead(21);
    digitalPinState[11] = digitalRead(22);
    digitalPinState[12] = digitalRead(23);
    analogPinState[0] = analogRead(A0);
    analogPinState[1] = analogRead(A1);
    analogPinState[2] = analogRead(A2);
    analogPinState[3] = analogRead(A3);
    analogPinState[4] = analogRead(A4);
    analogPinState[5] = analogRead(A5);
    analogPinState[6] = analogRead(A6);
    analogPinState[7] = analogRead(A7);
  } else {
    // TODO: find what pins changed from last state and save/report them to lead scout
  }
}
