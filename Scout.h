#ifndef LIB_PINOCCIO_SCOUT_H_
#define LIB_PINOCCIO_SCOUT_H_

#define P_MAX_BACKPACKS 3
#define P_MAX_LEAD_SCOUTS 3

#include <Pinoccio.h>
#include <utility/Backpack.h>
#include <Wire.h>

#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/halFuelGauge.h"
#include "utility/halRgbLed.h"
#include "PBBP.h"

class PinoccioScout : public PinoccioClass {

  public:
    PinoccioScout();
    ~PinoccioScout();

    void setup();
    void loop();

    bool isBatteryCharging();
    int getBatteryPercentage();
    int getBatteryVoltage();
    bool isBatteryAlarmTriggered();

    void enableBackpackVcc();
    void disableBackpackVcc();
    bool isBackpackVccEnabled();

    bool isLeadScout();

    void startDigitalStateChangeEvents();
    void stopDigitalStateChangeEvents();
    void startAnalogStateChangeEvents();
    void stopAnalogStateChangeEvents();
    void setStateChangeEventPeriods(uint32_t digitalInterval, uint32_t analogInterval);
    void saveState();

    PBBP bp;

    void (*digitalPinEventHandler)(uint8_t pin, uint8_t value);
    void (*analogPinEventHandler)(uint8_t pin, uint16_t value);
    void (*batteryPercentageEventHandler)(uint8_t value);
    void (*batteryVoltageEventHandler)(uint8_t value);
    void (*batteryChargingEventHandler)(uint8_t value);
    void (*batteryAlarmTriggeredEventHandler)(uint8_t value);
    void (*temperatureEventHandler)(uint8_t value);

    uint8_t digitalPinState[7];
    uint16_t analogPinState[8];
    uint8_t batteryPercentage;
    uint16_t batteryVoltage;
    bool isBattCharging;
    bool isBattAlarmTriggered;
    uint8_t temperature;

    bool eventVerboseOutput;

  protected:
    void checkStateChange();

    uint16_t leadScoutAddresses[P_MAX_LEAD_SCOUTS];
    Backpack* backpacks[P_MAX_BACKPACKS];

    bool isVccEnabled;
    bool isStateSaved;

    SYS_Timer_t digitalStateChangeTimer;
    SYS_Timer_t analogStateChangeTimer;
};

extern PinoccioScout Scout;
static void scoutDigitalStateChangeTimerHandler(SYS_Timer_t *timer);
static void scoutAnalogStateChangeTimerHandler(SYS_Timer_t *timer);

#endif
