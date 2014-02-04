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

    // Does not take effect until autoConnectHq() is called
    bool wifiConfig(const char *ssid, const char *passphrase);
    // Takes effect immediately
    bool wifiDhcp(const char *hostname);
    // Takes effect immediately
    bool wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns);

    // (Re-)connects the wifi and HQ connection
    bool autoConnectHq();

    void printAPs(Print& p);
    void printProfiles(Print& p);
    void printCurrentNetworkStatus(Print& p);
    bool printTime(Print& p);
    void printFirmwareVersions(Print& p);

    bool isAPConnected();
    bool isHQConnected();

    bool dnsLookup(Print &p, const char *host);
    bool ping(Print &p, const char *host);

    /** Run a command and print the results */
    bool runDirectCommand(Print& p, const char *command);

    bool goToSleep();
    bool wakeUp();

    GSClient client;
  protected:
    GSModule gs;

    // Event handlers
    static void onAssociate(void *data);
    static void onNcmConnect(void *data, GSCore::cid_t cid);
    static void onNcmDisconnect(void *data);
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_
