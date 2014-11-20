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
#include "backpacks/Backpacks.h"
#include "../Backpack.h"
#include <GS.h>

namespace pinoccio {
  class WifiBackpack : public Backpack {

    public:
      WifiBackpack();
      virtual ~WifiBackpack();

      bool setup(BackpackInfo *info);
      bool init();
      void loop();

      // Does not take effect until autoConnectHq() is called
      bool wifiConfig(const char *ssid, const char *passphrase);
      // Takes effect immediately
      bool wifiDhcp(const char *hostname);
      // Takes effect immediately
      bool wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns);

      // (Re-)associates the wifi (and afterwards autoconnects to HQ)
      bool associate();
      void disassociate();

      bool printAPs(Print& p);
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

      void setVerbose(bool flag);

      GSTcpClient client;

      uint16_t apConnCount;
      uint16_t hqConnCount;
      uint32_t connectedAt;
      bool indicate = 0;

      GSModule gs;

    protected:

      // Event handler
      static void onAssociate(void *data);

      // Creates the TCP connection to HQ, to be called while already
      // associated
      bool connectToHq();

      // flag to know when we're trying to associate
      bool associating = false;
  };
} // namespace pinoccio

#endif // LIB_PINOCCIO_WIFI_BACKPACK_H_
