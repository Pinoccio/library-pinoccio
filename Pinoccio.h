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
#include "utility/fastDelegate.h"
#include "avr/sleep.h"

typedef FastDelegate1<> FuncDelegate1;

class Pinoccio {

  public:
    Pinoccio();
    ~Pinoccio();

    void setup();
    void loop();

    float getTemperature();
    uint32_t getRandomNumber();

    uintptr_t getFreeMemory();

    void meshSendMessage(uint16_t destinationAddr, uint8_t* message, uint8_t length, uint8_t options=0);
    void meshListenForMessages();

    //bool publish(char* topic, char* payload, int size);
    //bool subscribe(char*, bool (*handler)(NWK_DataInd_t *msg));

    NWK_DataInd_t meshResponse;

  protected:
    uint16_t randomNumber;
};

#endif