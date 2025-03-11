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

/**
 * PinConfig represents the configuration of a pin, containing a Mode
 * and one or more Flags. The internal representation of a PinConfig and
 * a Mode is an integer, and it can be freely and automatically cast to
 * and from integers, to allow e.g. switch statements, or (bitwise)
 * arithmetic.
 */
struct PinConfig {
    struct Mode {
      Mode(uint8_t value) : value(value) { };
      operator uint8_t() {return value;}

      /** Is the pin active as input or output? */
      bool active() {return value > RESERVED;}
      /** Is the pin configured as input? */
      bool input() {return value == INPUT_DIGITAL || value == INPUT_ANALOG;}
      /** Is the pin configured as output? */
      bool output() {return value == OUTPUT_DIGITAL || value == OUTPUT_PWM;}

      enum {
        UNSET = 0,
        DISABLED = 1,
        DISCONNECTED = 2,
        RESERVED = 3,
        INPUT_DIGITAL = 4,
        INPUT_ANALOG = 5,
        OUTPUT_DIGITAL = 6,
        OUTPUT_PWM = 7,

        MASK = 0x7,
      };

      private:
        uint8_t value;
    };

    struct Flag {
      enum {
        PULLUP = 0x08,
      };
    };

    PinConfig() : value(Mode::UNSET) { };
    PinConfig(uint8_t value) : value(value) { };
    operator uint8_t() {return value;}

    /** Return the mode within this config. */
    Mode mode() {
      return (value & Mode::MASK);
    }


  private:
    uint8_t value;
};

class PinoccioScout : public PinoccioClass {

  public:
    PinoccioScout();
    ~PinoccioScout();

    void setup(const char *sketchName = "Custom", const char *sketchRevision = "unknown", int32_t sketchBuild = -1);
    void loop();

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
    void setStateChangeEventCycle(uint32_t digitalInterval, uint32_t analogInterval, uint32_t peripheralInterval);
    void saveState();

    int8_t getRegisterPinMode(uint8_t pin);
    PinConfig getPinConfig(uint8_t pin);
    bool makeInput(uint8_t pin, bool enablePullup=true);
    bool makeOutput(uint8_t pin);
    bool makePWM(uint8_t pin);
    bool makeDisabled(uint8_t pin);
    void makeUnsetDisconnected();
    bool setPinConfig(uint8_t pin, PinConfig config, uint8_t outvalue = 0);
    bool isDigitalPin(uint8_t pin);
    bool isAnalogPin(uint8_t pin);
    bool isPWMPin(uint8_t pin);
    bool pinWrite(uint8_t pin, uint8_t value);
    bool pinWritePWM(uint8_t pin, uint8_t value);
    uint16_t pinRead(uint8_t pin);
    int8_t getPinFromName(const char* name);
    const __FlashStringHelper* getNameForPin(uint8_t pin);
    const __FlashStringHelper* getNameForPinMode(PinConfig::Mode mode);
    bool isPinReserved(uint8_t pin);

    bool updatePinState(uint8_t pin, int16_t val, PinConfig config);

    void (*digitalPinEventHandler)(uint8_t pin, int16_t value, int8_t mode);
    void (*analogPinEventHandler)(uint8_t pin, int16_t value, int8_t mode);
    void (*batteryPercentageEventHandler)(uint8_t value);
    void (*batteryChargingEventHandler)(uint8_t value);
    void (*batteryAlarmTriggeredEventHandler)(uint8_t value);
    void (*temperatureEventHandler)(int8_t tempC, int8_t tempF);

    struct PinState {
      uint16_t value:10;
      uint8_t config:6;
    };

    PinState pinStates[NUM_DIGITAL_PINS];

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

  protected:
    uint32_t lastIndicate = 0;
    void checkStateChange();

    void doSleep(bool pastEnd);

    bool isVccEnabled;
    bool isStateSaved;
    bool isFactoryResetReady;

    SYS_Timer_t digitalStateChangeTimer;
    SYS_Timer_t analogStateChangeTimer;
    SYS_Timer_t peripheralStateChangeTimer;

    bool sleepPending;
    // The original sleep time, used to pass to the callback and to
    // re-sleep. The actual sleep time for the next sleep is stored by
    // SleepHandler instead.
    uint32_t sleepMs;
    char * postSleepFunction;
};

extern PinoccioScout Scout;

#endif
