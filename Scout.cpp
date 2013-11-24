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
  stateSaved = false;

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
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioScout::disableBackpackVcc() {
  digitalWrite(VCC_ENABLE, LOW);
}

bool PinoccioScout::isLeadScout() {
  enableBackpackVcc();
  delay(250);
  // for now, we ping Serial1 directly. Later, use PBBP
  Serial1.begin(115200);
  Serial1.println("AT");
  uint32_t time = millis();
  char buffer[2] = {0};
  uint8_t ctr;

  while (millis() - time < 1000) {
    if (Serial1.available()) {
      buffer[0] = buffer[1];
      buffer[1] = Serial1.read();
      buffer[2] = 0;

      if (strncmp((const char*)buffer, "AT", 2) == 0) {
        return true;
      }
    }
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