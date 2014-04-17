#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>
#include "backpacks/Backpacks.h"
#include <math.h>
#include <avr/eeprom.h>

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer);
static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer);
static void scoutPeripheralStateChangeTimerHandler(SYS_Timer_t *timer);

PinoccioScout Scout;

PinoccioScout::PinoccioScout() {
  digitalPinEventHandler = 0;
  analogPinEventHandler = 0;
  batteryPercentageEventHandler = 0;
  batteryChargingEventHandler = 0;
  batteryAlarmTriggeredEventHandler = 0;
  temperatureEventHandler = 0;

  digitalStateChangeTimer.interval = 50;
  digitalStateChangeTimer.mode = SYS_TIMER_PERIODIC_MODE;
  digitalStateChangeTimer.handler = scoutDigitalStateChangeTimerHandler;

  analogStateChangeTimer.interval = 60000;
  analogStateChangeTimer.mode = SYS_TIMER_PERIODIC_MODE;
  analogStateChangeTimer.handler = scoutAnalogStateChangeTimerHandler;

  peripheralStateChangeTimer.interval = 60000;
  peripheralStateChangeTimer.mode = SYS_TIMER_PERIODIC_MODE;
  peripheralStateChangeTimer.handler = scoutPeripheralStateChangeTimerHandler;

  eventVerboseOutput = false;
  isFactoryResetReady = false;
}

PinoccioScout::~PinoccioScout() { }

void PinoccioScout::setup(const char *sketchName, const char *sketchRevision, int32_t sketchBuild) {
  PinoccioClass::setup(sketchName, sketchRevision, sketchBuild);

  pinMode(CHG_STATUS, INPUT_PULLUP);
  pinMode(BATT_ALARM, INPUT_PULLUP);
  pinMode(VCC_ENABLE, OUTPUT);

  disableBackpackVcc();
  delay(100);
  enableBackpackVcc();

  RgbLed.turnOff();
  Wire.begin();
  HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%
  Backpacks::setup();

  saveState();
  handler.setup();

  startDigitalStateChangeEvents();
  startAnalogStateChangeEvents();
  startPeripheralStateChangeEvents();

  Shell.setup();
}

void PinoccioScout::loop() {
  PinoccioClass::loop();
  Shell.loop();
  handler.loop();

  if (isLeadScout()) {
    wifi.loop();
  }
}

void PinoccioScout::delay(unsigned long ms) {
  Serial.println("not safe, disabled");
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

bool PinoccioScout::isBatteryAlarmTriggered() {
  return isBattAlarmTriggered;
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
  return Backpacks::isModelPresent(0x0001);
}

bool PinoccioScout::factoryReset() {
  if (!isFactoryResetReady) {
    isFactoryResetReady = true;
    return false;
  } else {
    return true;
  }
}

void PinoccioScout::startDigitalStateChangeEvents() {
  SYS_TimerStart(&digitalStateChangeTimer);
}

void PinoccioScout::stopDigitalStateChangeEvents() {
  SYS_TimerStop(&digitalStateChangeTimer);
}

void PinoccioScout::startAnalogStateChangeEvents() {
  SYS_TimerStart(&analogStateChangeTimer);
}

void PinoccioScout::stopAnalogStateChangeEvents() {
  SYS_TimerStop(&analogStateChangeTimer);
}

void PinoccioScout::startPeripheralStateChangeEvents() {
  SYS_TimerStart(&peripheralStateChangeTimer);
}

void PinoccioScout::stopPeripheralStateChangeEvents() {
  SYS_TimerStop(&peripheralStateChangeTimer);
}

void PinoccioScout::setStateChangeEventCycle(uint32_t digitalInterval, uint32_t analogInterval, uint32_t peripheralInterval) {
  stopDigitalStateChangeEvents();
  digitalStateChangeTimer.interval = digitalInterval;
  startDigitalStateChangeEvents();

  stopAnalogStateChangeEvents();
  analogStateChangeTimer.interval = analogInterval;
  startAnalogStateChangeEvents();

  stopPeripheralStateChangeEvents();
  peripheralStateChangeTimer.interval = peripheralInterval;
  startPeripheralStateChangeEvents();
}

void PinoccioScout::saveState() {
  for (int i=0; i<7; i++) {
    if (isPinReserved(i+2)) {
      digitalPinMode[i] = -2;
      digitalPinState[i] = -1;
    } else {
      makeDisabled(i+2);
    }
  }

  for (int i=0; i<8; i++) {
    if (isPinReserved(i+A0)) {
      analogPinMode[i] = -2;
      analogPinState[i] = -1;
    } else {
      makeDisabled(i+A0);
    }
  }

  batteryPercentage = constrain(HAL_FuelGaugePercent(), 0, 100);
  batteryVoltage = HAL_FuelGaugeVoltage();
  isBattCharging = (digitalRead(CHG_STATUS) == LOW);
  isBattAlarmTriggered = (digitalRead(BATT_ALARM) == LOW);
  temperature = this->getTemperature();
}

int8_t PinoccioScout::getRegisterPinMode(uint8_t pin) {
  if (isPinReserved(pin)) {
    return -1;
  }
  if ((~(*portModeRegister(digitalPinToPort(pin))) & digitalPinToBitMask(pin)) &&
      (~(*portOutputRegister(digitalPinToPort(pin))) & digitalPinToBitMask(pin))) {
    return INPUT; // 0
  }
  if ((*portModeRegister(digitalPinToPort(pin)) & digitalPinToBitMask(pin))) {
    return OUTPUT; // 1
  }
  if ((~(*portModeRegister(digitalPinToPort(pin))) & digitalPinToBitMask(pin)) &&
      (*portOutputRegister(digitalPinToPort(pin)) & digitalPinToBitMask(pin))) {
    return INPUT_PULLUP; // 2
  }
}

int8_t PinoccioScout::getPinMode(uint8_t pin) {
  if (isDigitalPin(pin)) {
    return digitalPinMode[pin-2];
  }

  if (isAnalogPin(pin)) {
    return analogPinMode[pin-A0];
  }
}

bool PinoccioScout::makeInput(uint8_t pin, bool enablePullup) {
  if (isPinReserved(pin)) {
    return false;
  }

  uint8_t mode = enablePullup ? INPUT_PULLUP : INPUT;
  pinMode(pin, mode);

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = mode;
    digitalPinState[pin-2] = Scout.pinRead(pin);
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-A0] = mode;
    analogPinState[pin-A0] = Scout.pinRead(pin);
  }

  return true;
}

bool PinoccioScout::makeOutput(uint8_t pin) {
  if (isPinReserved(pin)) {
    return false;
  }

  pinMode(pin, OUTPUT);

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = OUTPUT;
    digitalPinState[pin-2] = Scout.pinRead(pin);
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-A0] = OUTPUT;
    analogPinState[pin-A0] = Scout.pinRead(pin);
  }

  return true;
}

bool PinoccioScout::makeDisabled(uint8_t pin) {
  if (isPinReserved(pin)) {
    return false;
  }

  pinMode(pin, INPUT);  // input-no-pullup, for lowest power draw

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = -1;
    digitalPinState[pin-2] = -1;
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-A0] = -1;
    analogPinState[pin-A0] = -1;
  }

  return true;
}

bool PinoccioScout::setMode(uint8_t pin, uint8_t mode) {
  if (isPinReserved(pin)) {
    return false;
  }

  pinMode(pin, mode);

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = mode;
    digitalPinState[pin-2] = Scout.pinRead(pin);
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-A0] = mode;
    analogPinState[pin-A0] = Scout.pinRead(pin);
  }

  return true;
}

bool PinoccioScout::isDigitalPin(uint8_t pin) {
  if (pin >= 2 && pin <= 8) {
    return true;
  }
  return false;
}

bool PinoccioScout::isAnalogPin(uint8_t pin) {
  if (pin >= A0 && pin <= A0 + NUM_ANALOG_INPUTS) {
    return true;
  }
  return false;
}

bool PinoccioScout::pinWrite(uint8_t pin, uint8_t value) {
  if (isPinReserved(pin)) {
    return false;
  }

  if (Scout.isDigitalPin(pin)) {
    Scout.makeOutput(pin);
    digitalWrite(pin, value);
    digitalPinState[pin-2] = value;
  }
  if (Scout.isAnalogPin(pin)) {
    Scout.makeOutput(pin);
    digitalWrite(pin, value);
    analogPinState[pin-A0] = value;
  }

  return true;
}

uint16_t PinoccioScout::pinRead(uint8_t pin) {
  if (Scout.isDigitalPin(pin)) {
    return digitalRead(pin);
  } else if (Scout.isAnalogPin(pin)) {
    if (Scout.getPinMode(pin) == INPUT) {
      return analogRead(pin-A0);
    } else {
      return digitalRead(pin);
    }
  } else {
    return 0;
  }
}

int8_t PinoccioScout::getPinFromName(const char* name) {
  uint8_t pin;

  if (name[0] == 'd') {
    pin = atoi(name+1);
  }

  if (name[0] == 'a') {
    pin = atoi(name+1) + A0;
  }

  if (!isDigitalPin(pin) && !isAnalogPin(pin)) {
    return -1;
  }

  return pin;
}

bool PinoccioScout::isPinReserved(uint8_t pin) {
  return (Backpacks::used_pins & Pbbe::LogicalPin(pin).mask());
}

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer) {
  int8_t val;
  int8_t mode;

  if (Scout.digitalPinEventHandler != 0) {
    for (int i=0; i<7; i++) {
      int pin = i+2;

      // Skip disabled/reserved pins
      if (Scout.digitalPinMode[i] < 0) {
        Scout.digitalPinState[i] = -1;
        continue;
      }

      val = Scout.pinRead(pin);
      mode = Scout.getRegisterPinMode(pin);

      if (Scout.digitalPinState[i] != val) {
        if (Scout.eventVerboseOutput) {
          Serial.print(F("Running: digitalPinEventHandler("));
          Serial.print(pin);
          Serial.print(F(","));
          Serial.print(val);
          Serial.print(F(","));
          Serial.print(mode);
          Serial.println(F(")"));
        }
        Scout.digitalPinState[i] = val;
        Scout.digitalPinMode[i] = mode;
        Scout.digitalPinEventHandler(pin, val, mode);
      }
    }
  }
}

static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer) {
  int16_t val;
  int8_t mode;

  if (Scout.analogPinEventHandler != 0) {
    for (int i=0; i<NUM_ANALOG_INPUTS; i++) {

      // Skip disabled/reserved pins
      if (Scout.analogPinMode[i] < 0) {
        Scout.analogPinState[i] = -1;
        continue;
      }

      val = Scout.pinRead(i+A0); // explicit digital pins until we can update core
      mode = Scout.getRegisterPinMode(i+A0);

      if (Scout.analogPinState[i] != val) {
        if (Scout.eventVerboseOutput) {
          Serial.print(F("Running: analogPinEventHandler("));
          Serial.print(i);
          Serial.print(F(","));
          Serial.print(val);
          Serial.print(F(","));
          Serial.print(mode);
          Serial.println(F(")"));
        }
        Scout.analogPinState[i] = val;
        Scout.analogPinMode[i] = mode;
        Scout.analogPinEventHandler(i, val, mode);
      }
    }
  }
}

static void scoutPeripheralStateChangeTimerHandler(SYS_Timer_t *timer) {
  uint16_t val;

  if (Scout.batteryPercentageEventHandler != 0) {
    val = constrain(HAL_FuelGaugePercent(), 0, 100);
    if (Scout.batteryPercentage != val) {
      if (Scout.eventVerboseOutput) {
        Serial.print(F("Running: batteryPercentageEventHandler("));
        Serial.print(val);
        Serial.println(F(")"));
      }
      Scout.batteryPercentage = val;
      Scout.batteryPercentageEventHandler(val);
    }
  }

  if (Scout.batteryChargingEventHandler != 0) {
    val = (digitalRead(CHG_STATUS) == LOW);
    if (Scout.isBattCharging != val) {
      if (Scout.eventVerboseOutput) {
        Serial.print(F("Running: batteryChargingEventHandler("));
        Serial.print(val);
        Serial.println(F(")"));
      }
      Scout.isBattCharging = val;
      Scout.batteryChargingEventHandler(val);
    }
  }

  if (Scout.batteryAlarmTriggeredEventHandler != 0) {
    val = (digitalRead(BATT_ALARM) == LOW);
    if (Scout.isBattAlarmTriggered != val) {
      if (Scout.eventVerboseOutput) {
        Serial.print(F("Running: batteryAlarmTriggeredEventHandler("));
        Serial.print(val);
        Serial.println(F(")"));
      }
      Scout.isBattAlarmTriggered = val;
      Scout.batteryAlarmTriggeredEventHandler(val);
    }
  }

  if (Scout.temperatureEventHandler != 0) {
    val = Scout.getTemperature();
    if (Scout.temperature != val) {
      if (Scout.eventVerboseOutput) {
        Serial.print(F("Running: temperatureEventHandler("));
        Serial.print(val);
        Serial.println(F(")"));
      }
      Scout.temperature = val;
      Scout.temperatureEventHandler(val);
    }
  }
}
