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

    bool apConfig(const char *ssid, const char *passphrase, String ip, String port);
    bool apConnect();
    void printAPs();
    void printProfiles();
    void printCurrentNetworkStatus();

    bool dnsLookup(const char *host);
    bool ping(const char *host);

    bool runDirectCommand(const char *command);

    bool goToSleep();
    bool wakeUp();

    bool getTime();

    PinoccioWifiClient client;

  protected:
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_