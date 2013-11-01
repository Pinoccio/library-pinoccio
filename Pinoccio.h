#ifndef LIB_PINOCCIO_H_
#define LIB_PINOCCIO_H_

//#define PINOCCIO_DEBUG

#ifdef PINOCCIO_DEBUG
#  define D(x) x
#else
#  define D(x)
#endif

#define SYS_SECURITY_MODE                   0
#define NWK_BUFFERS_AMOUNT                  4
#define NWK_MAX_ENDPOINTS_AMOUNT            4
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   800  // ms
#define NWK_ENABLE_MULTICAST
// End - Specifics for the LWM library

#include <Arduino.h>

#include "bitlash.h"
#include "src/bitlash.h"

#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/sysTimer.h"
#include "utility/halSleep.h"
#include "utility/halTemperature.h"
#include "utility/meshRequest.h"
#include "avr/sleep.h"

class PinoccioClass {

  public:
    PinoccioClass();
    ~PinoccioClass();

    void disableShell();
    void setup();
    void loop();

    void goToSleep(uint32_t sleepForMs);
        
    int8_t getTemperature();
    uint32_t getRandomNumber();
    void loadSettingsFromEeprom();
    
    void meshSetRadio(const uint16_t theAddress, const uint16_t thePanId=0x4567, const uint8_t theChannel=0x1a);
    void meshSetPower(const uint8_t theTxPower);
    void meshSetSecurityKey(const char *key);
    void meshSendMessage(MeshRequest request);
    void meshListen(uint8_t endpoint, bool (*handler)(NWK_DataInd_t *ind));
    
    void meshJoinGroup(uint16_t groupAddress);
    void meshLeaveGroup(uint16_t groupAddress);
      
    uint16_t getAddress();
    uint16_t getPanId();
    uint8_t getChannel();
    uint8_t getTxPower();
    const char* getTxPowerDb();
    
  protected:
    uint16_t randomNumber;
    bool shellEnabled;
    uint16_t address;
    uint16_t panId;
    uint8_t channel;
    uint8_t txPower;
};

extern PinoccioClass Pinoccio;

#endif