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


class PinoccioScout : public PinoccioClass {

  public:
    PinoccioScout();
    ~PinoccioScout();

    void setup();
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

    int8_t getPinMode(uint8_t pin);
    bool isDigitalPin(uint8_t pin);
    bool isAnalogPin(uint8_t pin);

    void (*digitalPinEventHandler)(uint8_t pin, uint8_t value);
    void (*analogPinEventHandler)(uint8_t pin, uint16_t value);
    void (*batteryPercentageEventHandler)(uint8_t value);
    void (*batteryVoltageEventHandler)(uint8_t value);
    void (*batteryChargingEventHandler)(uint8_t value);
    void (*batteryAlarmTriggeredEventHandler)(uint8_t value);
    void (*temperatureEventHandler)(uint8_t value);

    int8_t digitalPinState[7];
    int16_t analogPinState[8];
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
