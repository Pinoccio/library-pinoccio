/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include "../../util/StringBuffer.h"
#include "../../key/key.h"
#include "../../Scout.h"
#include "../../Shell.h"
#include "WifiModule.h"

using namespace pinoccio;

WifiModule WifiModule::instance;

/****************************\
 *   WIFI SHELL HANDLERS    *
\****************************/

static StringBuffer wifiReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[%s,%s]]",
          keyMap("wifi", 0),
          keyMap("connected", 0),
          keyMap("hq", 0),
          WifiModule::instance.bp().isAPConnected() ? "true" : "false",
          WifiModule::instance.bp().isHQConnected() ? "true" : "false");
  return Scout.handler.report(report);
}

static numvar wifiReport(void) {
  speol(wifiReportHQ());
  return 1;
}

static numvar wifiStatus(void) {
  if (getarg(0) > 0 && getarg(1) == 1) {
    WifiModule::instance.bp().printProfiles(Serial);
  } else {
    WifiModule::instance.bp().printFirmwareVersions(Serial);
    WifiModule::instance.bp().printCurrentNetworkStatus(Serial);
  }
  return 1;
}

static numvar wifiList(void) {
  if (!WifiModule::instance.bp().printAPs(Serial)) {
    speol(F("Error: Scan failed"));
    return 0;
  }
  return 1;
}

static numvar wifiConfig(void) {
  if (!checkArgs(1, 2, F("usage: wifi.config(\"wifiAPName\", \"wifiAPPassword\")"))) {
    return 0;
  }

  if (!WifiModule::instance.bp().wifiConfig((const char *)getstringarg(1), (const char *)getstringarg(2))) {
    speol(F("Error: saving WifiModule::instance.bp().configuration data failed"));
  }
  return 1;
}

static numvar wifiDhcp(void) {
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!WifiModule::instance.bp().wifiDhcp(host)) {
    speol(F("Error: saving WifiModule::instance.bp().configuration data failed"));
  }
  return 1;
}

static numvar wifiStatic(void) {
  if (!checkArgs(4, F("usage: wifi.static(\"ip\", \"netmask\", \"gateway\", \"dns\")"))) {
    return 0;
  }

  IPAddress ip, nm, gw, dns;

  if (!GSCore::parseIpAddress(&ip, (const char *)getstringarg(1))) {
    speol(F("Error: Invalid IP address"));
    return 0;
  }

  if (!GSCore::parseIpAddress(&nm, (const char *)getstringarg(2))) {
    speol(F("Error: Invalid netmask"));
    return 0;
  }

  if (!GSCore::parseIpAddress(&gw, (const char *)getstringarg(3))) {
    speol(F("Error: Invalid gateway"));
    return 0;
  }

  if (!GSCore::parseIpAddress(&dns, (const char *)getstringarg(3))) {
    speol(F("Error: Invalid dns server"));
    return 0;
  }

  if (!WifiModule::instance.bp().wifiStatic(ip, nm, gw, dns)) {
    speol(F("Error: saving WifiModule::instance.bp().configuration data failed"));
    return 0;
  }
  return 1;
}

static numvar wifiDisassociate(void) {
  WifiModule::instance.bp().disassociate();
  return 1;
}

static numvar wifiReassociate(void) {
  // This restart the NCM
  return WifiModule::instance.bp().autoConnectHq();
}

static numvar wifiCommand(void) {
  if (!checkArgs(1, F("usage: wifi.command(\"command\")"))) {
    return 0;
  }
  if (!WifiModule::instance.bp().runDirectCommand(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi direct command failed"));
  }
  return 1;
}

static numvar wifiPing(void) {
  if (!checkArgs(1, F("usage: wifi.ping(\"hostname\")"))) {
    return 0;
  }
  if (!WifiModule::instance.bp().ping(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi ping command failed"));
  }
  return 1;
}

static numvar wifiDNSLookup(void) {
  if (!checkArgs(1, F("usage: wifi.dnslookup(\"hostname\")"))) {
    return 0;
  }
  if (!WifiModule::instance.bp().dnsLookup(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi DNS lookup command failed"));
  }
  return 1;
}

static numvar wifiGetTime(void) {
  if (!WifiModule::instance.bp().printTime(Serial)) {
     speol(F("Error: Wi-Fi NTP time lookup command failed"));
  }
  return 1;
}

static numvar wifiSleep(void) {
  if (!WifiModule::instance.bp().goToSleep()) {
     speol(F("Error: Wi-Fi sleep command failed"));
  }
  return 1;
}

static numvar wifiWakeup(void) {
  if (!WifiModule::instance.bp().wakeUp()) {
     speol(F("Error: Wi-Fi wakeup command failed"));
  }
  return 1;
}

static numvar wifiVerbose(void) {
  // TODO
  return 1;
}

static numvar wifiStats(void) {
  sp(F("Number of connections to AP since boot: "));
  speol(WifiModule::instance.bp().apConnCount);
  sp(F("Number of connections to HQ since boot: "));
  speol(WifiModule::instance.bp().hqConnCount);
}

/****************************\
 *   MODULE CLASS STUFF     *
\****************************/

const __FlashStringHelper *WifiModule::name() const {
  return F("wifi");
}

bool WifiModule::enable() {
  Shell.addFunction("wifi.report", wifiReport);
  Shell.addFunction("wifi.status", wifiStatus);
  Shell.addFunction("wifi.list", wifiList);
  Shell.addFunction("wifi.config", wifiConfig);
  Shell.addFunction("wifi.dhcp", wifiDhcp);
  Shell.addFunction("wifi.static", wifiStatic);
  Shell.addFunction("wifi.reassociate", wifiReassociate);
  Shell.addFunction("wifi.disassociate", wifiDisassociate);
  Shell.addFunction("wifi.command", wifiCommand);
  Shell.addFunction("wifi.ping", wifiPing);
  Shell.addFunction("wifi.dnslookup", wifiDNSLookup);
  Shell.addFunction("wifi.gettime", wifiGetTime);
  Shell.addFunction("wifi.sleep", wifiSleep);
  Shell.addFunction("wifi.wakeup", wifiWakeup);
  Shell.addFunction("wifi.verbose", wifiVerbose);
  Shell.addFunction("wifi.stats", wifiStats);

  _bp = new WifiBackpack();
  if (!_bp)
    return false;

  _bp->setup() && _bp->autoConnectHq();
}

void WifiModule::loop() {
  _bp->loop();
}
