/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_WIFI_MODULE_H_
#define LIB_PINOCCIO_WIFI_MODULE_H_

#include <Pinoccio.h>
#include <GS.h>
#include "../Module.h"

namespace pinoccio {
  class WifiModule : public Module {

    public:
      bool load();
      const __FlashStringHelper *name() const;
      char *hq_host;
      uint16_t hq_port;
      bool verbose;
      void loop();

      // Does not take effect until autoConnectHq() is called
      bool wifiConfig(const char *ssid, const char *passphrase);
      // Takes effect immediately
      bool wifiDhcp(const char *hostname);
      // Takes effect immediately
      bool wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns);

      // (Re-)connects the wifi and HQ connection
      bool reassociate();
      void disassociate();

      bool printAPs(Print& p);
      void printProfiles(Print& p);
      void printCurrentNetworkStatus(Print& p);
      bool printTime(Print& p);
      void printFirmwareVersions(Print& p);
      int getHardwareMajorRevision();
      int getHardwareMinorRevision();

      bool isAPConnected();

      bool dnsLookup(Print &p, const char *host);
      bool ping(Print &p, const char *host);

      /** Run a command and print the results */
      bool runDirectCommand(Print& p, const char *command);

      bool goToSleep();
      bool wakeUp();

      uint16_t apConnCount;
      uint16_t hqConnCount;

      GSModule gs;

    protected:

      // Event handlers
      static void onAssociate(void *data);

    // Ensure there is always exactly one instance by declaring it here
    // and making our constructor private
    private:
      using Module::Module;
    public:
      static WifiModule instance;
  };
} // namespace pinoccio
#endif
