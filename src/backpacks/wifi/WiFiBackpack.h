/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_WIFI_BACKPACK_H_
#define LIB_PINOCCIO_WIFI_BACKPACK_H_

#include <Pinoccio.h>
#include "../Backpack.h"
#include "../../hq/HqHandler.h"
#include <GS.h>
#include <telehash.h>

class WiFiBackpack : public Backpack {

  public:
    WiFiBackpack();
    ~WiFiBackpack();

    bool setup();
    bool init();
    void loop();
    
    // tries to send packet if network is available
    void sendPacket(packet_t p);

    // Does not take effect until autoConnectHq() is called
    bool wifiConfig(const char *ssid, const char *passphrase);
    // Takes effect immediately
    bool wifiDhcp(const char *hostname);
    // Takes effect immediately
    bool wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns);

    // (Re-)connects the wifi
    bool associate();
    void disassociate();

    bool printAPs(Print& p);
    void printProfiles(Print& p);
    void printCurrentNetworkStatus(Print& p);
    bool printTime(Print& p);
    void printFirmwareVersions(Print& p);

    bool isAPConnected();
    bool isAvailable();

    bool dnsLookup(Print &p, const char *host);
    bool ping(Print &p, const char *host);

    /** Run a command and print the results */
    bool runDirectCommand(Print& p, const char *command);

    bool goToSleep();
    bool wakeUp();
    
    uint16_t apConnCount;
    bool available;
    
    GSModule gs;
    
  protected:

    GSUdpServer server;

    // Event handlers
    static void onAssociate(void *data);
    static void onNcmConnect(void *data, GSCore::cid_t cid);
    static void onNcmDisconnect(void *data);
};

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_
