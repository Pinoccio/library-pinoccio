#ifndef LIB_PINOCCIO_WIFI_BACKPACK_H_
#define LIB_PINOCCIO_WIFI_BACKPACK_H_

#include <Pinoccio.h>
#include <utility/Backpack.h>

#include "utility/webGainspan.h"
#include "utility/webWifi.h"
#include "utility/webWifiServer.h"
#include "utility/webWifiClient.h"
#include "utility/Flash.h"

class WiFiBackpack : public Backpack {

  public:
    WiFiBackpack();
    ~WiFiBackpack();

    bool setup();
    bool init();
    void loop();

    bool apConfig(const char *ssid, const char *passphrase);
    void printAPs();
    void printProfiles();
    void printCurrentNetworkStatus();
    bool connectToAP();
    bool connectToHQ(IPAddress server, uint16_t port);

    void dnsLookup(const char *host);
    void ping(const char *host);

    bool runDirectCommand(const char *command);

    bool goToSleep();
    bool wakeUp();

    PinoccioWifiClient client;

  protected:
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_