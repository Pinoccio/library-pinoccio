#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>
#include <Backpacks.h>
#include <math.h>
#include <avr/eeprom.h>

PinoccioScout Scout;

PinoccioScout::PinoccioScout() {
  digitalPinEventHandler = 0;
  analogPinEventHandler = 0;
  batteryPercentageEventHandler = 0;
  batteryVoltageEventHandler = 0;
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

void PinoccioScout::setup() {
  PinoccioClass::setup();

  pinMode(CHG_STATUS, INPUT_PULLUP);
  pinMode(BATT_ALARM, INPUT_PULLUP);
  pinMode(VCC_ENABLE, OUTPUT);

  disableBackpackVcc();
  delay(100);
  enableBackpackVcc();

  RgbLed.turnOff();

  Backpacks::setup();
  Shell.setup();
  handler.setup();

  Wire.begin();
  HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%

  saveState();
  startDigitalStateChangeEvents();
  startAnalogStateChangeEvents();
  startPeripheralStateChangeEvents();
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
  unsigned long target = millis() + ms;
  while ((long)(millis() - target) < 0) {
    loop();
  }
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

void PinoccioScout::setStateChangeEventPeriods(uint32_t digitalInterval, uint32_t analogInterval, uint32_t peripheralInterval) {
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
  Pbbe::LogicalPin::mask_t used = Backpacks::used_pins;

  digitalPinState[0] = (used & Pbbe::LogicalPin(2).mask()) ? -1 : digitalRead(2);
  digitalPinState[1] = (used & Pbbe::LogicalPin(3).mask()) ? -1 : digitalRead(3);
  digitalPinState[2] = (used & Pbbe::LogicalPin(4).mask()) ? -1 : digitalRead(4);
  digitalPinState[3] = (used & Pbbe::LogicalPin(5).mask()) ? -1 : digitalRead(5);
  digitalPinState[4] = (used & Pbbe::LogicalPin(6).mask()) ? -1 : digitalRead(6);
  digitalPinState[5] = (used & Pbbe::LogicalPin(7).mask()) ? -1 : digitalRead(7);
  digitalPinState[6] = (used & Pbbe::LogicalPin(8).mask()) ? -1 : digitalRead(8);
  analogPinState[0] = (used & Pbbe::LogicalPin(A0).mask()) ? -1 : analogRead(0);
  analogPinState[1] = (used & Pbbe::LogicalPin(A1).mask()) ? -1 : analogRead(1);
  analogPinState[2] = (used & Pbbe::LogicalPin(A2).mask()) ? -1 : analogRead(2);
  analogPinState[3] = (used & Pbbe::LogicalPin(A3).mask()) ? -1 : analogRead(3);
  analogPinState[4] = (used & Pbbe::LogicalPin(A4).mask()) ? -1 : analogRead(4);
  analogPinState[5] = (used & Pbbe::LogicalPin(A5).mask()) ? -1 : analogRead(5);
  analogPinState[6] = (used & Pbbe::LogicalPin(A6).mask()) ? -1 : analogRead(6);
  analogPinState[7] = (used & Pbbe::LogicalPin(A7).mask()) ? -1 : analogRead(7);
  batteryPercentage = constrain(HAL_FuelGaugePercent(), 0, 100);
  batteryVoltage = HAL_FuelGaugeVoltage();
  isBattCharging = (digitalRead(CHG_STATUS) == LOW);
  isBattAlarmTriggered = (digitalRead(BATT_ALARM) == LOW);
  temperature = this->getTemperature();
}

int8_t PinoccioScout::getPinMode(uint8_t pin) {
  // TODO: This requires a lot of expensive bitshifting, perhaps we can
  // move this check up into the loop that calls getPinMode to require
  // only one shift per loop iteration?
  if (Backpacks::used_pins & Pbbe::LogicalPin(pin).mask()) {
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

bool PinoccioScout::isDigitalPin(uint8_t pin) {
  if (pin >= 2 && pin <= 8) {
    return true;
  }
  return false;
}

bool PinoccioScout::isAnalogPin(uint8_t pin) {
  if (pin >= 24 && pin <= 31) {
    return true;
  }
  return false;
}

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer) {
  uint16_t val;

  // TODO: This can likely be optimized by hitting the pin registers directly
  if (Scout.digitalPinEventHandler != 0) {
    for (int i=0; i<7; i++) {
      int pin = i+2;

      // Skip pins that don't have pull-ups enabled or are reserved by backpacks
      if (Scout.getPinMode(pin) < 1) {
        Scout.digitalPinState[i] = -1;
        continue;
      }

      val = digitalRead(pin);
      if (Scout.digitalPinState[i] != val) {
        if (Scout.eventVerboseOutput) {
          sp("Running: digitalPinEventHandler(");
          sp(pin);
          sp(",");
          sp(val);
          speol(")");
        }
        Scout.digitalPinState[i] = val;
        Scout.digitalPinEventHandler(pin, val);
      }
    }
  }
}

static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer) {
  uint16_t val;

  if (Scout.analogPinEventHandler != 0) {
    for (int i=0; i<8; i++) {
      val = analogRead(i); // explicit digital pins until we can update core
      if (Scout.analogPinState[i] != val) {
        if (Scout.eventVerboseOutput) {
          sp("Running: analogPinEventHandler(");
          sp(i);
          sp(",");
          sp(val);
          speol(")");
        }
        Scout.analogPinState[i] = val;
        Scout.analogPinEventHandler(i, val);
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
        sp("Running: batteryPercentageEventHandler(");
        sp(val);
        speol(")");
      }
      Scout.batteryPercentage = val;
      Scout.batteryPercentageEventHandler(val);
    }
  }

  if (Scout.batteryVoltageEventHandler != 0) {
    val = HAL_FuelGaugeVoltage();
    if (Scout.batteryVoltage != val) {
      if (Scout.eventVerboseOutput) {
        sp("Running: batteryVoltageEventHandler(");
        sp(val);
        speol(")");
      }
      Scout.batteryVoltage = val;
      Scout.batteryVoltageEventHandler(val);
    }
  }

  if (Scout.batteryChargingEventHandler != 0) {
    val = (digitalRead(CHG_STATUS) == LOW);
    if (Scout.isBattCharging != val) {
      if (Scout.eventVerboseOutput) {
        sp("Running: batteryChargingEventHandler(");
        sp(val);
        speol(")");
      }
      Scout.isBattCharging = val;
      Scout.batteryChargingEventHandler(val);
    }
  }

  if (Scout.batteryAlarmTriggeredEventHandler != 0) {
    val = (digitalRead(BATT_ALARM) == LOW);
    if (Scout.isBattAlarmTriggered != val) {
      if (Scout.eventVerboseOutput) {
        sp("Running: batteryAlarmTriggeredEventHandler(");
        sp(val);
        speol(")");
      }
      Scout.isBattAlarmTriggered = val;
      Scout.batteryAlarmTriggeredEventHandler(val);
    }
  }

  if (Scout.temperatureEventHandler != 0) {
    val = Scout.getTemperature();
    if (Scout.temperature != val) {
      if (Scout.eventVerboseOutput) {
        sp("Running: temperatureEventHandler(");
        sp(val);
        speol(")");
      }
      Scout.temperature = val;
      Scout.temperatureEventHandler(val);
    }
  }
}
