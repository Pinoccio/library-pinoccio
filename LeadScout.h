#ifndef LIB_PINOCCIO_LEADSCOUT_H_
#define LIB_PINOCCIO_LEADSCOUT_H_

#include "Pinoccio.h"

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

class PinoccioLeadScout : public PinoccioScout {

  public:
    PinoccioLeadScout();
    ~PinoccioLeadScout();

    void init();
    void loop();
    
    bool publishScoutToServer(uint16_t scoutAddress, char* topic, char* payload, int size);
    bool subscribeScoutToServer(uint16_t scoutAddress, char* topic);
    
    bool publishTroopMetadataToServer(char* topic, char* payload, int size);
    
  protected:
    Client netClient;
    mqttClient mqtt;
  
    Scout scouts[NWK_ROUTE_TABLE_SIZE];
};

extern PinoccioLeadScout LeadScout;

#endif