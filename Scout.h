#ifndef LIB_PINOCCIO_SCOUT_H_
#define LIB_PINOCCIO_SCOUT_H_

#define P_MAX_BACKPACKS 3
#define P_MAX_LEAD_SCOUTS 3

#include <Pinoccio.h>
#include <Backpack.h>

#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/halFuelGauge.h"
#include "utility/halRgbLed.h"

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

  protected:
    uint16_t leadScoutAddresses[P_MAX_LEAD_SCOUTS];   
    Backpack* backpacks[P_MAX_BACKPACKS];
};

extern PinoccioScout Scout;

#endif