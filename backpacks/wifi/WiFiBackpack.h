#ifndef LIB_PINOCCIO_WIFI_BACKPACK_H_
#define LIB_PINOCCIO_WIFI_BACKPACK_H_

#include <Pinoccio.h>
#include <backpacks/Backpack.h>

#include "utility/webGainspan.h"
#include "utility/webWifi.h"
#include "utility/webWifiServer.h"
#include "utility/webWifiClient.h"
#include "utility/Flash.h"

class WiFiBackpack : public Backpack {

  public:
    WiFiBackpack();
    ~WiFiBackpack();

    void setup();
    void loop();

    void config();
    void printAccessPoints();

    bool connectToAp();
    bool connectToHQ();



  protected:
    PinoccioWifiClient client;
    WIFI_PROFILE profile;
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_