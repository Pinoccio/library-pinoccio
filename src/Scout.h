/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_SCOUT_H_
#define LIB_PINOCCIO_SCOUT_H_

#include <Pinoccio.h>
#include <Shell.h>
#include <ScoutHandler.h>
#include "modules/ModuleHandler.h"
#include "backpack-bus/PBBP.h"
#include "util/Callback.h"
#include <Wire.h>
#include <lwm.h>

#include "lwm/phy/phy.h"
#include "lwm/hal/hal.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "peripherals/halFuelGauge.h"
#include "peripherals/halRgbLed.h"
#include "util/radio_state_t.h"

// This is a temporary hack to check the result of snprintf and print an
// error
/*
#define snprintf(buf, size, ...) do { \
  if (size <= snprintf(buf, size, __VA_ARGS__)) { \
    Serial.print("snprintf overflow "); \
    Serial.print(__FILE__); \
    Serial.print(":"); \
    Serial.println(__LINE__); \
    Serial.flush(); \
  } \
} while(0)
*/

// TODO: remove these when this class is put in the pinoccio namespace
using pinoccio::ScoutHandler;
using pinoccio::CallbackList;

class PinoccioScout : public PinoccioClass {

  public:
    PinoccioScout();
    ~PinoccioScout();

    void setup(const char *sketchName = "Custom", const char *sketchRevision = "unknown", int32_t sketchBuild = -1);
    void loop();

    bool isSleeping();

    bool isBatteryCharging();
    int getBatteryPercentage();
    int getBatteryVoltage();
    bool isBatteryAlarmTriggered();
    bool isBatteryConnected();
    
    int8_t getTemperatureC();
    int8_t getTemperatureF();

    void enableBackpackVcc();
    void disableBackpackVcc();
    bool isBackpackVccEnabled();

    bool isLeadScout();
    bool factoryReset();
    void reboot();

    void startDigitalStateChangeEvents();
    void stopDigitalStateChangeEvents();
    void startAnalogStateChangeEvents();
    void stopAnalogStateChangeEvents();
    void startPeripheralStateChangeEvents();
    void stopPeripheralStateChangeEvents();
    void startScheduleSleepTimer();
    void stopScheduleSleepTimer();
    void startWakeTimer();
    void stopWakeTimer();
    void setStateChangeEventCycle(uint32_t digitalInterval, uint32_t analogInterval, uint32_t peripheralInterval);
    void saveState();
    void setWakeMs(uint32_t wakePeriodMs);
    uint32_t getWakeMs();

    int8_t getRegisterPinMode(uint8_t pin);
    int8_t getPinMode(uint8_t pin);
    bool makeInput(uint8_t pin, bool enablePullup=true);
    bool makeOutput(uint8_t pin);
    bool makePWM(uint8_t pin);
    bool makeDisabled(uint8_t pin);
    void makeUnsetDisconnected();
    bool setMode(uint8_t pin, int8_t mode);
    bool isDigitalPin(uint8_t pin);
    bool isAnalogPin(uint8_t pin);
    bool isPWMPin(uint8_t pin);
    bool isInputPin(uint8_t pin);
    bool isOutputPin(uint8_t pin);
    bool pinWrite(uint8_t pin, uint8_t value);
    bool pinWritePWM(uint8_t pin, uint8_t value);
    uint16_t pinRead(uint8_t pin);
    int8_t getPinFromName(const char* name);
    const __FlashStringHelper* getNameForPin(uint8_t pin);
    const __FlashStringHelper* getNameForPinMode(int8_t mode);
    bool isPinReserved(uint8_t pin);

    bool updatePinState(uint8_t pin, int16_t val, int8_t mode);

    void (*digitalPinEventHandler)(uint8_t pin, int16_t value, int8_t mode);
    void (*analogPinEventHandler)(uint8_t pin, int16_t value, int8_t mode);
    void (*batteryPercentageEventHandler)(uint8_t value);
    void (*batteryChargingEventHandler)(uint8_t value);
    void (*batteryAlarmTriggeredEventHandler)(uint8_t value);
    void (*temperatureEventHandler)(int8_t tempC, int8_t tempF);

    int16_t pinStates[NUM_DIGITAL_PINS];
    int8_t pinModes[NUM_DIGITAL_PINS];

    uint8_t batteryPercentage;
    uint16_t batteryVoltage;
    bool isBattCharging;
    bool isBattAlarmTriggered;
    uint8_t temperature;

    CallbackList<void, bool> toggleBackpackVccCallbacks;

    bool eventVerboseOutput;
    bool eventsStopped;

    PBBP bp;
    ScoutHandler handler;
    
    uint32_t now = 0; // set every loop
    uint8_t indicate = 0; // how often to signal status

    // Schedule a sleep that lasts until now + ms. The optional bitlash
    // command is executed after the sleep. A previous sleep can be
    // canceled by passing 0, NULL. The command passed in will be
    // copied, so it does not have to remain valid.
    void scheduleSleep(uint32_t ms, const char *cmd);

    // sleep basd on global mesh time
    void scheduleSleep2(const char *cmd);
    void cancelSleep2();

    void sleepRadio();
    void wakeRadio();

    //should probably be protected
    void internalScheduleSleep();
    radio_state_t radioState;

    enum {
      PINMODE_DISCONNECTED = -4,
      PINMODE_UNSET = -3,
      PINMODE_RESERVED = -2,
      PINMODE_DISABLED = -1,
      PINMODE_INPUT = 0,
      PINMODE_OUTPUT = 1,
      PINMODE_INPUT_PULLUP = 2,
      PINMODE_PWM = 3,
    };
  protected:
    uint32_t lastIndicate = 0;

    void doSleep();

    bool isVccEnabled;
    bool isStateSaved;
    bool isFactoryResetReady;

    SYS_Timer_t digitalStateChangeTimer;
    SYS_Timer_t analogStateChangeTimer;
    SYS_Timer_t peripheralStateChangeTimer;
    SYS_Timer_t scheduleSleepTimer;
    SYS_Timer_t wakeTimer;

    bool sleepPending;
    bool automatedSleep;
    // The original sleep time, used to pass to the callback and to
    // re-sleep. The actual sleep time for the next sleep is stored by
    // SleepHandler instead.
    uint32_t sleepMs;
    char * postSleepFunction;
};

extern PinoccioScout Scout;

#endif
