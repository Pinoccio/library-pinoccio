#ifndef LIB_PINOCCIO_H_
#define LIB_PINOCCIO_H_

#define PINOCCIO_DEBUG

#ifdef PINOCCIO_DEBUG
#  define D(x) x
#else
#  define D(x) 
#endif

#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/sysTimer.h"
#include "utility/halSleep.h"
#include "utility/halTemperature.h"
#include "utility/halRgbLed.h"
#include "utility/webGainspan.h"
#include "utility/webWifi.h"
#include "utility/webWifiServer.h"
#include "utility/webWifiClient.h"
#include "utility/mqttClient.h"

// typedef struct NWK_DataReq_t sendMessage;
// typedef struct NWK_DataInd_t receiveMessage;

class PinoccioClass {

  public:
    PinoccioClass();
    ~PinoccioClass();

    void alive();
    void init();
    // TODO void initMesh();
    void loop();

    float getTemperature();
    bool isBatteryCharging();
    float getBatteryVoltage();
    void setRandomNumber(uint16_t number);
    
    bool publish(char* topic, char* payload, int size);
    bool subscribe(char*, bool (*handler)(NWK_DataInd_t *msg));
    
    bool sendMessage(NWK_DataReq_t *msg);
    bool listenForMessage(uint8_t id, bool (*handler)(NWK_DataInd_t *msg));
    
  protected:
    uint16_t randomNumber;
};

extern PinoccioClass Pinoccio;

#endif