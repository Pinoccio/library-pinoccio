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
    float getBatteryVoltage();

    void enableBackpackVcc();
    void disableBackpackVcc();
    bool isBackpackVccEnabled();

    bool isLeadScout();

    PBBP bp;

  protected:
    void checkStateChange();

    uint16_t leadScoutAddresses[P_MAX_LEAD_SCOUTS];
    Backpack* backpacks[P_MAX_BACKPACKS];

    uint8_t batteryPercent;
    bool vccEnabled;

    bool stateSaved;
    uint8_t digitalPinState[13];
    uint8_t analogPinState[8];
};

extern PinoccioScout Scout;

#endif
