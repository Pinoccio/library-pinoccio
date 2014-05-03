#ifndef LIB_PINOCCIO_WIFI_BACKPACK_H_
#define LIB_PINOCCIO_WIFI_BACKPACK_H_

#include <Pinoccio.h>
#include "../Backpack.h"
#include <GS.h>

class WiFiBackpack : public Backpack {

  public:
    WiFiBackpack();
    ~WiFiBackpack();

    bool setup();
    bool init();
    void loop();

    // Does not take effect until autoOnline() is called
    bool wifiConfig(const char *ssid, const char *passphrase);
    // Takes effect immediately
    bool wifiDhcp(const char *hostname);
    // Takes effect immediately
    bool wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns);

    // (Re-)connects the wifi
    bool autoOnline();
    void disassociate();

    bool printAPs(Print& p);
    void printProfiles(Print& p);
    void printCurrentNetworkStatus(Print& p);
    bool printTime(Print& p);
    void printFirmwareVersions(Print& p);

    bool isAPConnected();

    bool dnsLookup(Print &p, const char *host);
    bool ping(Print &p, const char *host);

    /** Run a command and print the results */
    bool runDirectCommand(Print& p, const char *command);

    bool goToSleep();
    bool wakeUp();

    GSUdpServer server;
    
    uint16_t apConnCount;
    uint16_t hqConnCount;
    
    void (*onOnline)(void);
    void (*onOffline)(void);
    
    GSModule gs;
    
  protected:

    // Event handlers
    static void onAssociate(void *data);
    static void onDisassociate(void *data);
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_
