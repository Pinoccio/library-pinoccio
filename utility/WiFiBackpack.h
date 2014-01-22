#ifndef LIB_PINOCCIO_WIFI_BACKPACK_H_
#define LIB_PINOCCIO_WIFI_BACKPACK_H_

#include <Pinoccio.h>
#include <utility/Backpack.h>
#include <GS.h>

class WiFiBackpack : public Backpack {

  public:
    WiFiBackpack();
    ~WiFiBackpack();

    bool setup();
    bool init();
    void loop();

    bool wifiConfig(const char *ssid, const char *passphrase);
    bool wifiDhcp(const char *hostname);
    bool wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns);
    bool autoConnectHq();

    void printAPs(Print& p);
    void printProfiles(Print& p);
    void printCurrentNetworkStatus(Print& p);
    bool printTime(Print& p);
    void printFirmwareVersions(Print& p);

    bool dnsLookup(Print &p, const char *host);
    bool ping(Print &p, const char *host);

    /** Run a command and print the results */
    bool runDirectCommand(Print& p, const char *command);

    bool goToSleep();
    bool wakeUp();

    GSClient client;
  protected:
    GSModule gs;
    // Was leadHQConnect already called?
    bool hqConnected = false;
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_
