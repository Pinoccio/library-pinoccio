#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>
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

  // Give the slaves on the backpack bus a bit of time to start up. 1ms
  // seems to be enough, but let's be generous.
  delay(5);
  bp.begin(BACKPACK_BUS);
  if (!bp.enumerate()) {
    /*
    sp("Backpack enumeration failed: ");
    bp.printLastError(Serial);
    speol();
    */
  }

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
  for (uint8_t i = 0; i < bp.num_slaves; ++i) {
    if (bp.slave_ids[i][1] == 0 &&
        bp.slave_ids[i][2] == 1)
      return true;
  }
  return false;
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

  // initialize all pins to the disabled mode (-1)
  for (int i=0; i<7; i++) {
    digitalPinState[i] = -1;
    digitalPinMode[i] = -1;
  }

  for (int i=0; i<8; i++) {
    analogPinState[i] = -1;
    analogPinMode[i] = -1;
  }

  batteryPercentage = constrain(HAL_FuelGaugePercent(), 0, 100);
  batteryVoltage = HAL_FuelGaugeVoltage();
  isBattCharging = (digitalRead(CHG_STATUS) == LOW);
  isBattAlarmTriggered = (digitalRead(BATT_ALARM) == LOW);
  temperature = this->getTemperature();
}

int8_t PinoccioScout::getRegisterPinMode(uint8_t pin) {
  // TODO: add this as a bp.isPinReserved(pin) method instead of hardwired
  if (Scout.isLeadScout() && pin >= 6 && pin <= 8) {
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
    return analogPinMode[pin-24];
  }
}

void PinoccioScout::makeInput(uint8_t pin, bool enablePullup) {
  int mode = INPUT;
  if (enablePullup) {
    mode = INPUT_PULLUP;
  }

  pinMode(pin, mode);

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = mode;
    digitalPinState[pin-2] = digitalRead(pin);
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-24] = mode;
    analogPinState[pin-24] = analogRead(pin);
  }
}

void PinoccioScout::makeOutput(uint8_t pin) {
  pinMode(pin, OUTPUT);

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = OUTPUT;
    digitalPinState[pin-2] = digitalRead(pin);
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-24] = OUTPUT;
    analogPinState[pin-24] = analogRead(pin);
  }
}

void PinoccioScout::makeDisabled(uint8_t pin) {
  pinMode(pin, INPUT);  // input-no-pullup, for lowest power draw

  if (isDigitalPin(pin)) {
    digitalPinMode[pin-2] = -1;
    digitalPinState[pin-2] = -1;
  }

  if (isAnalogPin(pin)) {
    analogPinMode[pin-24] = -1;
    analogPinState[pin-24] = -1;
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
      if (Scout.digitalPinMode[i] < 0) {
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
      int pin = i+24;

      // Skip pins that aren't enabled
      if (Scout.analogPinMode[i] < 0) {
        Scout.analogPinState[i] = -1;
        continue;
      }

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
