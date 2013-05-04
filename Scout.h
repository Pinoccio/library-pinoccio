#ifndef LIB_PINOCCIO_SCOUT_H_
#define LIB_PINOCCIO_SCOUT_H_

#include <Pinoccio.h>
#include <Backpack.h>

#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/sysTimer.h"
#include "utility/halSleep.h"
#include "utility/halTemperature.h"
#include "utility/halRgbLed.h"
#include "utility/mqttClient.h"

class PinoccioScout : public Pinoccio {

  public:
    PinoccioScout();
    ~PinoccioScout();

    void setup();
    void loop();

    bool isBatteryCharging();
    int getBatteryPercentage();

    void enableBackpackVcc();
    void disableBackpackVcc();

    friend class LeadScout;

  protected:
    uint16_t leadScoutAddress;   
    Backpack* backpacks[3];
};

extern PinoccioScout Scout;

#endif