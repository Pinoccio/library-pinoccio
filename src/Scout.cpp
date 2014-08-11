/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>
#include "backpacks/Backpacks.h"
#include "SleepHandler.h"
#include <math.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer);
static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer);
static void scoutPeripheralStateChangeTimerHandler(SYS_Timer_t *timer);

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof(*x))
#endif

// This allocates the same space for all strings, even when it's unused (since
// the struct must have a fixed size). The alternative, to put a char* in the
// struct causes the strings to be allocated outside of the string. This wastes
// less space, but makes putting the strings in PROGMEN hard. To prevent
// wasting a lot of space, some pin names have been abbreviated.
static struct PinInfo {
  const char name[5];
} const pinInfo[NUM_DIGITAL_PINS] PROGMEM = {
  [RX0] = { "rx0", },
  [TX0] = { "tx0", },
  [D2] = { "d2", },
  [D3] = { "d3", },
  [D4] = { "d4", },
  [D5] = { "d5", },
  [D6] = { "d6", },
  [D7] = { "d7", },
  [D8] = { "d8", },
  [SS] = { "ss", },
  [MOSI] = { "mosi", },
  [MISO] = { "miso", },
  [SCK] = { "sck", },
  [RX1] = { "rx1", },
  [TX1] = { "tx1", },
  [SCL] = { "scl", },
  [SDA] = { "sda", },
  [VCC_ENABLE] = { "vcc", },
  [BATT_ALERT] = { "batt", },
  [BACKPACK_BUS] = { "bkpk", },
  [CHG_STATUS] = { "chg", },
  [LED_BLUE] = { "ledb", },
  [LED_RED] = { "ledr", },
  [LED_GREEN] = { "ledg", },
  [A0] = { "a0", },
  [A1] = { "a1", },
  [A2] = { "a2", },
  [A3] = { "a3", },
  [A4] = { "a4", },
  [A5] = { "a5", },
  [A6] = { "a6", },
  [A7] = { "a7", },
};

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

  analogStateChangeTimer.interval = 1000;
  analogStateChangeTimer.mode = SYS_TIMER_PERIODIC_MODE;
  analogStateChangeTimer.handler = scoutAnalogStateChangeTimerHandler;

  peripheralStateChangeTimer.interval = 60000;
  peripheralStateChangeTimer.mode = SYS_TIMER_PERIODIC_MODE;
  peripheralStateChangeTimer.handler = scoutPeripheralStateChangeTimerHandler;

  eventVerboseOutput = false;
  isFactoryResetReady = false;

  sleepPending = false;
  postSleepFunction = NULL;
}

PinoccioScout::~PinoccioScout() { }

void PinoccioScout::setup(const char *sketchName, const char *sketchRevision, int32_t sketchBuild) {
  PinoccioClass::setup(sketchName, sketchRevision, sketchBuild);
  SleepHandler::setup();

  pinMode(CHG_STATUS, INPUT_PULLUP);
  pinMode(BATT_ALERT, INPUT_PULLUP);
  pinMode(VCC_ENABLE, OUTPUT);

  disableBackpackVcc();
  delay(100);
  enableBackpackVcc();

  Led.turnOff();
  Wire.begin();
  HAL_FuelGaugeConfig(20);   // Configure the MAX17048G's alert percentage to 20%
  Backpacks::setup();

  saveState();
  handler.setup();
  ModuleHandler::setup();

  startDigitalStateChangeEvents();
  startAnalogStateChangeEvents();
  startPeripheralStateChangeEvents();

  Shell.setup();
}

void PinoccioScout::loop() {
  bool canSleep = true;
  // TODO: Let other loop functions return some "cansleep" status as well

  PinoccioClass::loop();
  Shell.loop();
  handler.loop();
  ModuleHandler::loop();
  Backpacks::loop();

  if (sleepPending) {
    canSleep = canSleep && !NWK_Busy();

    // if remaining <= 0, we won't actually sleep anymore, but still
    // call doSleep to run the callback and clean up
    if (SleepHandler::scheduledTicksLeft() == 0)
      doSleep(true);
    else if (canSleep)
      doSleep(false);
  }
}

uint32_t PinoccioScout::uptime() {
  return SleepHandler::uptime().seconds;
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

bool PinoccioScout::isBatteryConnected() {
  bool start = digitalRead(CHG_STATUS);
  bool state = start;
  bool changed = false;

  for (int i=0; i<40; i++) {
    if ((state = digitalRead(CHG_STATUS)) != start) {
      changed = true;
    } else if (changed == true) {
      return false;
    }
    delay(1);
  }
  return true;
}

int8_t PinoccioScout::getTemperatureC() {
  return this->getTemperature();
}

int8_t PinoccioScout::getTemperatureF() {
  float f;
  f = round((1.8 * this->getTemperature()) + 32);
  return (uint32_t)f;
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
  return handler.isBridged || Backpacks::isModelPresent(0x0001);
}

bool PinoccioScout::factoryReset() {
  if (!isFactoryResetReady) {
    isFactoryResetReady = true;
    return false;
  } else {
    return true;
  }
}

void PinoccioScout::reboot() {
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
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
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    if (isPinReserved(i)) {
      pinModes[i] = PINMODE_RESERVED;
    } else {
      pinModes[i] = PINMODE_UNSET;
    }
    pinStates[i] = -1;
  }

  batteryPercentage = constrain(HAL_FuelGaugePercent(), 0, 100);
  batteryVoltage = HAL_FuelGaugeVoltage();
  isBattCharging = (digitalRead(CHG_STATUS) == LOW);
  isBattAlarmTriggered = (digitalRead(BATT_ALERT) == LOW);
  temperature = this->getTemperature();
}

int8_t PinoccioScout::getRegisterPinMode(uint8_t pin) {
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
  if (pin < NUM_DIGITAL_PINS)
    return pinModes[pin];
  return PINMODE_UNSET;
}

void PinoccioScout::makeUnsetDisconnected() {
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    if (getPinMode(i) == PINMODE_UNSET)
      setMode(i, PINMODE_DISCONNECTED);
  }
}

bool PinoccioScout::setMode(uint8_t pin, int8_t mode) {
  if (isPinReserved(pin)) {
    return false;
  }

  // pre-set initial values for mode change
  int value;
  switch (mode) {
    case PINMODE_DISABLED:
    case PINMODE_INPUT:
      pinMode(pin, INPUT);
      break;
    // On disconnected (floating) pins, enable a pullup. Without the
    // pullup, the input logic might switch between high and low all the
    // time, causing excessive power usage.
    case PINMODE_DISCONNECTED:
    case PINMODE_INPUT_PULLUP:
      pinMode(pin, INPUT_PULLUP);
      break;
    case PINMODE_OUTPUT:
    case PINMODE_PWM:
      pinMode(pin, OUTPUT);
      break;
    default:
      return false;
  }

  switch(mode) {
    case PINMODE_DISABLED:
      value = -1;
      break;
    case PINMODE_PWM:
      value = 0;
      break;
    case PINMODE_INPUT:
    case PINMODE_OUTPUT:
    case PINMODE_INPUT_PULLUP:
      value = pinRead(pin);
      break;
  }

  updatePinState(pin, value, mode);

  return true;
}

bool PinoccioScout::isDigitalPin(uint8_t pin) {
  return (pin >= D2 && pin <= D8);
}

bool PinoccioScout::isAnalogPin(uint8_t pin) {
  return (pin >= A0 && pin <= A0 + NUM_ANALOG_INPUTS);
}

bool PinoccioScout::isPWMPin(uint8_t pin) {
  return digitalPinHasPWM(pin);
}

bool PinoccioScout::isInputPin(uint8_t pin) {
  return (getPinMode(pin) == PINMODE_INPUT || getPinMode(pin) == PINMODE_INPUT_PULLUP) ? true : false;
}

bool PinoccioScout::isOutputPin(uint8_t pin) {
  return (getPinMode(pin) == PINMODE_OUTPUT || getPinMode(pin) == PINMODE_PWM) ? true : false;
}

bool PinoccioScout::pinWrite(uint8_t pin, uint8_t value) {
  if (isPinReserved(pin) || !isOutputPin(pin)) {
    return false;
  }

  if (getPinMode(pin) == PINMODE_PWM)
    analogWrite(pin, value);
  else
    digitalWrite(pin, value);

  updatePinState(pin, value, getPinMode(pin));

  return true;
}

uint16_t PinoccioScout::pinRead(uint8_t pin) {
  switch(getPinMode(pin)) {
    case PINMODE_PWM:
      return pinStates[pin];

    case PINMODE_INPUT:
    case PINMODE_INPUT_PULLUP:
      if (isAnalogPin(pin))
        return analogRead(pin - A0);
      else
        return digitalRead(pin);

    case PINMODE_OUTPUT:
      return digitalRead(pin);

    default:
      return 0;
  }
}

int8_t PinoccioScout::getPinFromName(const char* name) {
  for (uint8_t i = 0; i < lengthof(pinInfo); ++i) {
    if (strcasecmp_P(name, pinInfo[i].name) == 0)
      return i;
  }

  return -1;
}

const __FlashStringHelper* PinoccioScout::getNameForPin(uint8_t pin) {
  if (pin < lengthof(pinInfo))
    return reinterpret_cast<const __FlashStringHelper*>(pinInfo[pin].name);

  return NULL;
}

const __FlashStringHelper* PinoccioScout::getNameForPinMode(int8_t mode) {
  switch(mode) {
    case (PINMODE_DISCONNECTED):
      return F("disconnected");
    case (PINMODE_UNSET):
      return F("unset");
    case (PINMODE_RESERVED):
      return F("reserved");
    case (PINMODE_DISABLED):
      return F("disabled");
    case (PINMODE_INPUT):
      return F("input");
    case (PINMODE_OUTPUT):
      return F("output");
    case (PINMODE_INPUT_PULLUP):
      return F("input_pullup");
    case (PINMODE_PWM):
      return F("pwm");
    default:
      return NULL;
  }
}

bool PinoccioScout::isPinReserved(uint8_t pin) {
  if (Backpacks::used_pins & Pbbe::LogicalPin(pin).mask())
    return true;

  // TODO: Make this less hardcoded
  switch (pin) {
    case SCL:
    case SDA:
    case BATT_ALERT:
    case CHG_STATUS:
    case BACKPACK_BUS:
    case VCC_ENABLE:
    case RX0:
    case TX0:
    case LED_RED:
    case LED_GREEN:
    case LED_BLUE:
      return true;
    default:
      return false;
  }
}

bool PinoccioScout::updatePinState(uint8_t pin, int16_t val, int8_t mode) {
  if (pinStates[pin] != val || pinModes[pin] != mode) {
    pinStates[pin] = val;
    pinModes[pin] = mode;

    if (isDigitalPin(pin) && digitalPinEventHandler != 0) {
      if (eventVerboseOutput) {
        Serial.print(F("Running: digitalPinEventHandler("));
        Serial.print(pin);
        Serial.print(F(","));
        Serial.print(val);
        Serial.print(F(","));
        Serial.print(mode);
        Serial.println(F(")"));
      }
      digitalPinEventHandler(pin, val, mode);
    }

    if (isAnalogPin(pin) && analogPinEventHandler != 0) {
      if (eventVerboseOutput) {
        Serial.print(F("Running: analogPinEventHandler("));
        Serial.print(pin - A0);
        Serial.print(F(","));
        Serial.print(val);
        Serial.print(F(","));
        Serial.print(mode);
        Serial.println(F(")"));
      }
      analogPinEventHandler(pin - A0, val, mode);
    }

    return true;
  }
  return false;
}

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer) {
  if (Scout.digitalPinEventHandler != 0) {
    for (int i=2; i<9; i++) {
      int mode = Scout.getPinMode(i);
      if (mode >= 0) {
        int value = Scout.pinRead(i);
        Scout.updatePinState(i, value, mode);
      }
    }
  }
}

static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer) {
  if (Scout.analogPinEventHandler != 0) {
    for (int i=0; i<NUM_ANALOG_INPUTS; i++) {
      int mode = Scout.getPinMode(i+A0);
      if (mode >= 0) {
        int value = Scout.pinRead(i+A0);
        Scout.updatePinState(i+A0, value, mode);
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
    val = (digitalRead(BATT_ALERT) == LOW);
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
    int8_t tempC = Scout.getTemperatureC();
    if (Scout.temperature != tempC) {
      Scout.temperature = tempC;
      int8_t tempF = Scout.getTemperatureF();
      if (Scout.eventVerboseOutput) {
        Serial.print(F("Running: temperatureEventHandler("));
        Serial.print(tempC);
        Serial.print(",");
        Serial.print(tempF);
        Serial.println(F(")"));
      }
      Scout.temperatureEventHandler(tempC, tempF);
    }
  }
}

void PinoccioScout::scheduleSleep(uint32_t ms, char *func) {
  if (ms) {
    SleepHandler::scheduleSleep(ms);
    sleepPending = true;
  } else {
    sleepPending = false;
  }

  if (postSleepFunction)
    free(postSleepFunction);
  postSleepFunction = func;
  sleepMs = ms;
}

void PinoccioScout::doSleep(bool pastEnd) {
  // Copy the pointer, so the post command can set a new sleep
  // timeout again.
  char *func = postSleepFunction;
  postSleepFunction = NULL;
  sleepPending = false;

  if (!pastEnd) {
    NWK_SleepReq();

    // TODO: suspend more stuff? Wait for UART byte completion?

    SleepHandler::doSleep(true);
    NWK_WakeupReq();
  }

  // TODO: Allow ^C to stop running callbacks like this one
  if (func) {
    StringBuffer cmd(64, 16);
    uint32_t left = SleepHandler::ticksToMs(SleepHandler::scheduledTicksLeft());
    cmd += func;
    cmd += "(";
    cmd.appendSprintf("%lu", sleepMs);
    cmd.appendSprintf(",%lu", left);
    cmd += ")";

    uint32_t ret = doCommand((char*)cmd.c_str());

    if (!left || !ret || sleepPending) {
      // If the callback returned false, or it scheduled a new sleep or
      // we finished our previous sleep, then we're done with this
      // callback.
      free(func);
    } else {
      // If the callback returned true, and it did not schedule a new
      // sleep interval, and we're not done sleeping yet, this means we
      // should continue sleeping (though note that at least one loop
      // cycle is ran before actually sleeping again).
      sleepPending = true;
      postSleepFunction = func;
    }
  }
}
