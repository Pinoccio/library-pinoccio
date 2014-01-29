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

  eventVerboseOutput = false;
}

PinoccioScout::~PinoccioScout() { }

void PinoccioScout::setup() {
  PinoccioClass::setup();

  pinMode(CHG_STATUS, INPUT_PULLUP);
  pinMode(BATT_ALARM, INPUT_PULLUP);
  pinMode(VCC_ENABLE, OUTPUT);

  RgbLed.turnOff();

  enableBackpackVcc();
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

  handler.setup();
  Shell.setup();

  Wire.begin();
  HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%

  saveState();
  startDigitalStateChangeEvents();
  startAnalogStateChangeEvents();
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

void PinoccioScout::setStateChangeEventPeriods(uint32_t digitalInterval, uint32_t analogInterval) {
  stopDigitalStateChangeEvents();
  digitalStateChangeTimer.interval = digitalInterval;
  startDigitalStateChangeEvents();

  stopAnalogStateChangeEvents();
  analogStateChangeTimer.interval = analogInterval;
  startAnalogStateChangeEvents();
}

void PinoccioScout::saveState() {
  digitalPinState[0] = digitalRead(2);
  digitalPinState[1] = digitalRead(3);
  digitalPinState[2] = digitalRead(4);
  digitalPinState[3] = digitalRead(5);
  digitalPinState[4] = digitalRead(6);
  digitalPinState[5] = digitalRead(7);
  digitalPinState[6] = digitalRead(8);
  analogPinState[0] = analogRead(0); // pin 24
  analogPinState[1] = analogRead(1); // pin 25
  analogPinState[2] = analogRead(2); // pin 26
  analogPinState[3] = analogRead(3); // pin 27
  analogPinState[4] = analogRead(4); // pin 28
  analogPinState[5] = analogRead(5); // pin 29
  analogPinState[6] = analogRead(6); // pin 30
  analogPinState[7] = analogRead(7); // pin 31

  batteryPercentage = constrain(HAL_FuelGaugePercent(), 0, 100);
  batteryVoltage = HAL_FuelGaugeVoltage();
  isBattCharging = (digitalRead(CHG_STATUS) == LOW);
  isBattAlarmTriggered = (digitalRead(BATT_ALARM) == LOW);
  temperature = this->getTemperature();
}

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer) {
  uint16_t val;

  // TODO: This can likely be optimized by hitting the pin registers directly
  if (Scout.digitalPinEventHandler != 0) {
    for (int i=0; i<7; i++) {
      // Skip pins D0 an D1 (TX0 and RX0). TODO: Unhardcode this
      // Scout-specific detail
      int pin = i + 2;
      // Skip output mode pins
      if (*portModeRegister(digitalPinToPort(pin)) & digitalPinToBitMask(pin))
        continue;
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
  const uint8_t analogThreshold = 10;
  uint16_t val;

  if (Scout.analogPinEventHandler != 0) {
    for (int i=0; i<8; i++) {
      val = analogRead(i); // explicit digital pins until we can update core
      if (abs(Scout.analogPinState[i] - val) > analogThreshold) {
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
    val = HAL_MeasureTemperature();
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
