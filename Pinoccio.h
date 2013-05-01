#ifndef LIB_PINOCCIO_H_
#define LIB_PINOCCIO_H_

#define PINOCCIO_DEBUG

#ifdef PINOCCIO_DEBUG
#  define D(x) x
#else
#  define D(x)
#endif

// Start - Specifics for the LWM library
#define NWK_ENABLE_SECURITY
#define NWK_ENABLE_ROUTING
#define PHY_ENABLE_ENERGY_DETECTION
#define PHY_ENABLE_RANDOM_NUMBER_GENERATOR

#define NWK_BUFFERS_AMOUNT                  3
#define NWK_MAX_ENDPOINTS_AMOUNT            2
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   800  // ms
// End - Specifics for the LWM library

#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/sysTimer.h"
#include "utility/halSleep.h"
#include "utility/halTemperature.h"
#include "utility/halRgbLed.h"

#include "Backpack.h"

class PinoccioClass {

  public:
    PinoccioClass();
    ~PinoccioClass();

    void init();
    void loop();

    float getTemperature();

    bool isBatteryCharging();
    float getBatteryVoltage();

    void enableBackpackVcc();
    void disableBackpackVcc();
    
    uintptr_t getFreeMemory();

    void setRandomNumber(uint16_t number);

  protected:
    uint16_t randomNumber;
    Backpack backpacks[3];
};

#endif