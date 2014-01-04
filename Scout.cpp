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

  digitalPinEventHandler = 0;
  analogPinEventHandler = 0;
  batteryPercentEventHandler = 0;
  batteryVoltageEventHandler = 0;
  batteryChargingEventHandler = 0;
  temperatureEventHandler = 0;

  stateChangeTimer.interval = 100;
  stateChangeTimer.mode = SYS_TIMER_PERIODIC_MODE;
  stateChangeTimer.handler = scoutStateChangeTimerHandler;
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


  // Enumerate backpack bus
  if (!bp.enumerate()) {
    bp.printLastError(Serial);
    Serial.println();
  }

  saveState();
  startStateChangeEvents();
}

void PinoccioScout::loop() {
  PinoccioClass::loop();
}

bool PinoccioScout::isBatteryCharging() {
  return isBattCharging;
}

int PinoccioScout::getBatteryPercentage() {
  return batteryPercentage;
}

int PinoccioScout::getBatteryVoltage() {
  return batteryVoltage;
}

void PinoccioScout::enableBackpackVcc() {
  isVccEnabled = true;
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioScout::disableBackpackVcc() {
  isVccEnabled = false;
  digitalWrite(VCC_ENABLE, LOW);
}

bool PinoccioScout::isBackpackVccEnabled() {
  return isVccEnabled;
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

void PinoccioScout::startStateChangeEvents() {
  SYS_TimerStart(&stateChangeTimer);
}

void PinoccioScout::stopStateChangeEvents() {
  SYS_TimerStop(&stateChangeTimer);
}

void PinoccioScout::setStateChangeEventPeriod(uint32_t interval) {
  stopStateChangeEvents();
  stateChangeTimer.interval = interval;
  startStateChangeEvents();
}

void PinoccioScout::saveState() {
  digitalPinState[0] = digitalRead(2);
  digitalPinState[1] = digitalRead(3);
  digitalPinState[2] = digitalRead(4);
  digitalPinState[3] = digitalRead(5);
  digitalPinState[4] = digitalRead(6);
  digitalPinState[5] = digitalRead(7);
  digitalPinState[6] = digitalRead(8);
  analogPinState[0] = analogRead(A0); // pin 24
  analogPinState[1] = analogRead(A1); // pin 25
  analogPinState[2] = analogRead(A2); // pin 26
  analogPinState[3] = analogRead(A3); // pin 27
  analogPinState[4] = analogRead(A4); // pin 28
  analogPinState[5] = analogRead(A5); // pin 29
  analogPinState[6] = analogRead(A6); // pin 30
  analogPinState[7] = analogRead(A7); // pin 31

  batteryPercentage = constrain(HAL_FuelGaugePercent(), 0, 100);
  batteryVoltage = HAL_FuelGaugeVoltage();
  isBattCharging = (digitalRead(CHG_STATUS) == LOW);
  temperature = HAL_MeasureTemperature();
}

static void scoutStateChangeTimerHandler(SYS_Timer_t *timer) {
  const uint8_t analogThreshold = 10;
  uint16_t val;

  // TODO: This can likely be optimized by hitting the pin registers directly
  // Also, this should probably be moved to interrupts
  if (Scout.digitalPinEventHandler != 0) {
    for (uint8_t i=0; i<7; i++) {
      val = digitalRead(i+2);
      if (Scout.digitalPinState[i] != val) {
        Scout.digitalPinEventHandler(i+2, val);
        Scout.digitalPinState[i] = val;
      }
    }
  }

  if (Scout.analogPinEventHandler != 0) {
    for (uint8_t i=0; i<8; i++) {
      val = analogRead(i+24);
      if (abs(Scout.analogPinState[i] - val) > analogThreshold) {
        Scout.analogPinEventHandler(i+24, val);
        Scout.analogPinState[i] = val;
      }
    }
  }

  if (Scout.batteryPercentEventHandler != 0) {
    val = constrain(HAL_FuelGaugePercent(), 0, 100);
    if (Scout.batteryPercentage != val) {
      Scout.batteryPercentEventHandler(val);
      Scout.batteryPercentage = val;
    }
  }

  if (Scout.batteryVoltageEventHandler != 0) {
    val = HAL_FuelGaugeVoltage();
    if (Scout.batteryVoltage != val) {
      Scout.batteryVoltageEventHandler(val);
      Scout.batteryVoltage = val;
    }
  }

  if (Scout.batteryChargingEventHandler != 0) {
    val = (digitalRead(CHG_STATUS) == LOW);
    if (Scout.isBattCharging != val) {
      Scout.batteryChargingEventHandler(val);
      Scout.isBattCharging = val;
    }
  }

  if (Scout.temperatureEventHandler != 0) {
    val = HAL_MeasureTemperature();
    if (Scout.temperature != val) {
      Scout.temperatureEventHandler(val);
      Scout.temperature = val;
    }
  }
}
