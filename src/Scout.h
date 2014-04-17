#ifndef LIB_PINOCCIO_SCOUT_H_
#define LIB_PINOCCIO_SCOUT_H_

#include <Pinoccio.h>
#include <Shell.h>
#include <ScoutHandler.h>
#include "backpack-bus/PBBP.h"
#include "backpacks/wifi/WiFiBackpack.h"
#include <Wire.h>

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

class PinoccioScout : public PinoccioClass {

  public:
    PinoccioScout();
    ~PinoccioScout();

    void setup(const char *sketchName = "Custom", const char *sketchRevision = "unknown", int32_t sketchBuild = -1);
    void loop();
    void delay(unsigned long ms);

    bool isBatteryCharging();
    int getBatteryPercentage();
    int getBatteryVoltage();
    bool isBatteryAlarmTriggered();

    void enableBackpackVcc();
    void disableBackpackVcc();
    bool isBackpackVccEnabled();

    bool isLeadScout();
    bool factoryReset();

    void startDigitalStateChangeEvents();
    void stopDigitalStateChangeEvents();
    void startAnalogStateChangeEvents();
    void stopAnalogStateChangeEvents();
    void startPeripheralStateChangeEvents();
    void stopPeripheralStateChangeEvents();
    void setStateChangeEventCycle(uint32_t digitalInterval, uint32_t analogInterval, uint32_t peripheralInterval);
    void saveState();

    int8_t getRegisterPinMode(uint8_t pin);
    int8_t getPinMode(uint8_t pin);
    bool makeInput(uint8_t pin, bool enablePullup=true);
    bool makeOutput(uint8_t pin);
    bool makeDisabled(uint8_t pin);
    bool setMode(uint8_t pin, uint8_t mode);
    bool isDigitalPin(uint8_t pin);
    bool isAnalogPin(uint8_t pin);
    bool pinWrite(uint8_t pin, uint8_t value);
    uint16_t pinRead(uint8_t pin);
    int8_t getPinFromName(const char* name);
    bool isPinReserved(uint8_t pin);

    void (*digitalPinEventHandler)(uint8_t pin, int8_t value, int8_t mode);
    void (*analogPinEventHandler)(uint8_t pin, int16_t value, int8_t mode);
    void (*batteryPercentageEventHandler)(uint8_t value);
    void (*batteryChargingEventHandler)(uint8_t value);
    void (*batteryAlarmTriggeredEventHandler)(uint8_t value);
    void (*temperatureEventHandler)(uint8_t value);

    int8_t digitalPinState[7];
    int8_t digitalPinMode[7];
    int16_t analogPinState[8];
    int8_t analogPinMode[8];

    uint8_t batteryPercentage;
    uint16_t batteryVoltage;
    bool isBattCharging;
    bool isBattAlarmTriggered;
    uint8_t temperature;

    bool eventVerboseOutput;

    PBBP bp;
    WiFiBackpack wifi;
    PinoccioScoutHandler handler;

  protected:
    void checkStateChange();

    bool isVccEnabled;
    bool isStateSaved;
    bool isFactoryResetReady;

    SYS_Timer_t digitalStateChangeTimer;
    SYS_Timer_t analogStateChangeTimer;
    SYS_Timer_t peripheralStateChangeTimer;
};

extern PinoccioScout Scout;

#endif
