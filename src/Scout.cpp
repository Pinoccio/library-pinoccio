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

using namespace pinoccio;

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

  saveState();
  ModuleHandler::setup();

  Backpacks::setup();

  // start after so any backpacks are ready
  handler.setup();

  startDigitalStateChangeEvents();
  startAnalogStateChangeEvents();
  startPeripheralStateChangeEvents();

  // indicate before running custom scripts
  Led.setTorch();

  Shell.setup();

  // if Led is still torch'd (startup didn't change it) indicate post-startup
  if(Led.getRedValue() == Led.getRedTorchValue() && Led.getGreenValue() == Led.getGreenTorchValue() && Led.getBlueValue() == Led.getBlueTorchValue())
  {
    // disable it in a bit
    Led.blinkTorch(100);

    // if low power, red blink warning
    if(isBattAlarmTriggered && !isBattCharging)
    {
      Shell.eval("scout.delay",500,"led.red(50)",100,"led.red(50)",100,"led.red(50)");
    }
  }
  
}

void PinoccioScout::loop() {
  now = SleepHandler::uptime().seconds;

  bool canSleep = true;
  // TODO: Let other loop functions return some "cansleep" status as well

  PinoccioClass::loop();

  // every 5th second blink network status
  bool showStatus = (indicate && lastIndicate < now && (now % indicate == 0));
  if(showStatus)
  {
    Led.setRedValue(Led.getRedTorchValue(), false);
    Led.setGreenValue(Led.getGreenTorchValue(), false);
    Led.setBlueValue(Led.getBlueTorchValue(), false);

    NWK_RouteTableEntry_t *table = NWK_RouteTable();
    bool meshed = 0;
    for (int i=0; i < NWK_ROUTE_TABLE_SIZE; i++)
    {
      if (table[i].dstAddr != NWK_ROUTE_UNKNOWN) meshed = 1;
    }

    if(meshed)
    {
      lastIndicate = now;
    }

  }

  Shell.loop();
  handler.loop();
  ModuleHandler::loop();
  Backpacks::loop();

  if(showStatus)
  {
    Led.setRedValue(Led.getRedValue());
    Led.setGreenValue(Led.getGreenValue());
    Led.setBlueValue(Led.getBlueValue());
  }

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

bool PinoccioScout::isBatteryCharging() {
  return (digitalRead(CHG_STATUS) == LOW);
}

int PinoccioScout::getBatteryPercentage() {
  return constrain(HAL_FuelGaugePercent(), 0, 100);;
}

int PinoccioScout::getBatteryVoltage() {
  return HAL_FuelGaugeVoltage();
}

bool PinoccioScout::isBatteryAlarmTriggered() {
  return (digitalRead(BATT_ALERT) == LOW);
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
  toggleBackpackVccCallbacks.callAll(true);
}

void PinoccioScout::disableBackpackVcc() {
  isVccEnabled = false;
  toggleBackpackVccCallbacks.callAll(false);
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
    PinState *state = &pinStates[i];
    if (isPinReserved(i)) {
      state->config = PinConfig::Mode::RESERVED;
    } else {
      state->config = PinConfig::Mode::UNSET;
    }
  }

  batteryPercentage = this->getBatteryPercentage();
  batteryVoltage = this->getBatteryVoltage();
  isBattCharging = this->isBatteryCharging();
  isBattAlarmTriggered = this->isBatteryAlarmTriggered();
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

PinConfig PinoccioScout::getPinConfig(uint8_t pin) {
  if (pin < NUM_DIGITAL_PINS)
    return pinStates[pin].config;
  return PinConfig::Mode::UNSET;
}

void PinoccioScout::makeUnsetDisconnected() {
  for (int i=0; i<NUM_DIGITAL_PINS; i++) {
    if (getPinConfig(i).mode() == PinConfig::Mode::UNSET)
      setPinConfig(i, PinConfig::Mode::DISCONNECTED);
  }
}

bool PinoccioScout::setPinConfig(uint8_t pin, PinConfig config, uint8_t outvalue) {
  // Cannot change pins marked as reserved
  if (getPinConfig(pin).mode() == PinConfig::Mode::RESERVED)
    return false;

  // Pullup flag is only valid for input modes
  if ((config & PinConfig::Flag::PULLUP)
      && config.mode() != PinConfig::Mode::INPUT_DIGITAL
      && config.mode() != PinConfig::Mode::INPUT_ANALOG)
    return false;

  if (config.mode() == PinConfig::Mode::OUTPUT_PWM && !digitalPinHasPWM(pin))
    return false;

  if (config.mode() == PinConfig::Mode::INPUT_ANALOG && !isAnalogPin(pin))
    return false;

  uint16_t value;
  switch(config.mode()) {
    case PinConfig::Mode::INPUT_DIGITAL:
      pinMode(pin, INPUT);
      // Writing non-zero to an input pin enables the pullup
      digitalWrite(pin, config & PinConfig::Flag::PULLUP);
      value = digitalRead(pin);
      break;
    case PinConfig::Mode::INPUT_ANALOG:
      pinMode(pin, INPUT);
      // Writing non-zero to an input pin enables the pullup
      digitalWrite(pin, config & PinConfig::Flag::PULLUP);
      value = analogRead(pin);
      break;
    case PinConfig::Mode::OUTPUT_DIGITAL:
      value = outvalue;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, outvalue);
      break;
    case PinConfig::Mode::OUTPUT_PWM:
      value = outvalue;
      pinMode(pin, OUTPUT);
      analogWrite(pin, value);
    case PinConfig::Mode::DISABLED:
      // INPUT is effectively high-impedance
      value = 0;
      pinMode(pin, INPUT);
      break;
    case PinConfig::Mode::DISCONNECTED:
      // On disconnected (floating) pins, enable a pullup. Without the
      // pullup, the input logic might switch between high and low all
      // the time, causing excessive power usage.
      value = 0;
      pinMode(pin, INPUT_PULLUP);
      break;
    case PinConfig::Mode::UNSET:
    default:
      return false;
  }

  updatePinState(pin, value, config);

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

bool PinoccioScout::pinWrite(uint8_t pin, uint8_t value) {
  PinConfig config = getPinConfig(pin);
  switch(config.mode()) {
    case PinConfig::Mode::OUTPUT_DIGITAL:
      digitalWrite(pin, value);
      break;

    case PinConfig::Mode::OUTPUT_PWM:
      analogWrite(pin, value);
      break;

    default:
      return false;
  }

  updatePinState(pin, value, config);

  return true;
}

uint16_t PinoccioScout::pinRead(uint8_t pin) {
  PinConfig config = getPinConfig(pin);
  switch(config.mode()) {
    case PinConfig::Mode::INPUT_ANALOG:
      return analogRead(pin - A0);

    case PinConfig::Mode::INPUT_DIGITAL:
      return digitalRead(pin);

    case PinConfig::Mode::OUTPUT_DIGITAL:
      return digitalRead(pin);

    case PinConfig::Mode::OUTPUT_PWM:
      return pinStates[pin].value;

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

const __FlashStringHelper* PinoccioScout::getNameForPinMode(PinConfig::Mode mode) {
  switch(mode) {
    case (PinConfig::Mode::DISCONNECTED):
      return F("disconnected");
    case (PinConfig::Mode::UNSET):
      return F("unset");
    case (PinConfig::Mode::DISABLED):
      return F("disabled");
    case (PinConfig::Mode::RESERVED):
      return F("reserved");
    case (PinConfig::Mode::INPUT_DIGITAL):
      return F("input_digital");
    case (PinConfig::Mode::INPUT_ANALOG):
      return F("input_analog");
    case (PinConfig::Mode::OUTPUT_DIGITAL):
      return F("output_digital");
    case (PinConfig::Mode::OUTPUT_PWM):
      return F("output_pwm");
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

bool PinoccioScout::updatePinState(uint8_t pin, int16_t val, PinConfig config) {
  PinState *state = &pinStates[pin];
  if (state->value != val || state->config != config) {
    state->value = val;
    state->config = config;

    if (isDigitalPin(pin) && digitalPinEventHandler != 0) {
      if (eventVerboseOutput) {
        Serial.print(F("Running: digitalPinEventHandler("));
        Serial.print(pin);
        Serial.print(F(","));
        Serial.print(val);
        Serial.print(F(","));
        Serial.print(config);
        Serial.println(F(")"));
      }
      digitalPinEventHandler(pin, val, config);
    }

    if (isAnalogPin(pin) && analogPinEventHandler != 0) {
      if (eventVerboseOutput) {
        Serial.print(F("Running: analogPinEventHandler("));
        Serial.print(pin - A0);
        Serial.print(F(","));
        Serial.print(val);
        Serial.print(F(","));
        Serial.print(config);
        Serial.println(F(")"));
      }
      analogPinEventHandler(pin - A0, val, config);
    }

    return true;
  }
  return false;
}

static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer) {
  if (Scout.digitalPinEventHandler != 0) {
    for (int i=2; i<9; i++) {
      PinConfig config = Scout.getPinConfig(i);
      if (config.mode().active()) {
        int value = Scout.pinRead(i);
        Scout.updatePinState(i, value, config);
      }
    }
  }
}

static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer) {
  if (Scout.analogPinEventHandler != 0) {
    for (int i=0; i<NUM_ANALOG_INPUTS; i++) {
      PinConfig config = Scout.getPinConfig(i+A0);
      if (config.mode().active()) {
        int value = Scout.pinRead(i+A0);
        Scout.updatePinState(i+A0, value, config);
      }
    }
  }
}

static void scoutPeripheralStateChangeTimerHandler(SYS_Timer_t *timer) {
  uint16_t val;

  if (Scout.batteryPercentageEventHandler != 0) {
    val = Scout.getBatteryPercentage();
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
    val = Scout.isBatteryCharging();
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

void PinoccioScout::scheduleSleep(uint32_t ms, const char *func) {
  if (ms) {
    SleepHandler::scheduleSleep(ms);
    sleepPending = true;
  } else {
    sleepPending = false;
  }

  if (postSleepFunction)
    free(postSleepFunction);
  postSleepFunction = func ? strdup(func) : NULL;
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

    uint32_t ret = Shell.eval((char*)cmd.c_str());

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
