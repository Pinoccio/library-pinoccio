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
#include "../../util/Callback.h"
#include "../../key/key.h"
#include "../../Scout.h"
#include "../../Shell.h"
#include "../../SleepHandler.h"
#include "../Backpacks.h"
#include "WifiModule.h"

using namespace pinoccio;

WifiModule WifiModule::instance;

/****************************\
 *   WIFI SHELL HANDLERS    *
\****************************/
static bool checkbp() {
  if (!WifiModule::instance.bp()) {
    speol(F("Cannot access wifi backpack"));
    return false;
  }
  return true;
}

static StringBuffer wifiReportHQ(void) {
  StringBuffer report(100);
  if (!checkbp()) return 0;
  WifiBackpack *bp = WifiModule::instance.bp();
  uint32_t uptime = 0;
  if(bp->connectedAt) uptime = (SleepHandler::uptime().seconds - bp->connectedAt);
  report.appendSprintf("[%d,[%d,%d,%d,%d,%d],[%s,%s,%lu,%hu,%hu]]",
          keyMap("wifi", 0),
          keyMap("connected", 0),
          keyMap("hq", 0),
          keyMap("uptime", 0),
          keyMap("reset", 0),
          keyMap("total", 0),
          bp->isAPConnected() ? "true" : "false",
          bp->isHQConnected() ? "true" : "false",
          uptime,
          bp->hqConnCount,
          bp->apConnCount);

  return Scout.handler.report(report);
}

static numvar wifiReport(void) {
  speol(wifiReportHQ());
  return 1;
}

static numvar wifiStatus(void) {
  if (!checkbp()) return 0;

  if (getarg(0) > 0 && getarg(1) == 1) {
    WifiModule::instance.bp()->printProfiles(Serial);
  } else {
    WifiModule::instance.bp()->printFirmwareVersions(Serial);
    WifiModule::instance.bp()->printCurrentNetworkStatus(Serial);
  }
  return 1;
}

static numvar wifiList(void) {
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->printAPs(Serial)) {
    speol(F("Error: Scan failed"));
    return 0;
  }
  return 1;
}

static numvar wifiConfig(void) {
  if (!checkArgs(1, 2, F("usage: wifi.config(\"wifiAPName\", \"wifiAPPassword\")"))) {
    return 0;
  }

  char password[64];

  if (!checkbp()) return 0;

  if (getarg(0) == 2) {
    strncpy(password, (const char *)getstringarg(2), 64);
  } else {
    strncpy(password, "", 64);
  }

  if (!WifiModule::instance.bp()->wifiConfig((const char *)getstringarg(1), (const char *)password)) {
    speol(F("Error: saving WifiModule::instance.bp()->configuration data failed"));
  }
  return 1;
}

static numvar wifiDhcp(void) {
  if (!checkbp()) return 0;
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!WifiModule::instance.bp()->wifiDhcp(host)) {
    speol(F("Error: saving WifiModule::instance.bp()->configuration data failed"));
  }
  return 1;
}

static numvar wifiStatic(void) {
  if (!checkArgs(4, F("usage: wifi.static(\"ip\", \"netmask\", \"gateway\", \"dns\")"))) {
    return 0;
  }
  if (!checkbp()) return 0;

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

  if (!WifiModule::instance.bp()->wifiStatic(ip, nm, gw, dns)) {
    speol(F("Error: saving WifiModule::instance.bp()->configuration data failed"));
    return 0;
  }
  return 1;
}

static numvar wifiDisassociate(void) {
  if (!checkbp()) return 0;
  WifiModule::instance.bp()->disassociate();
  return 1;
}

static numvar wifiReassociate(void) {
  if (!checkbp()) return 0;
  // This restart the NCM
  return WifiModule::instance.bp()->associate();
}

static numvar wifiCommand(void) {
  if (!checkArgs(1, F("usage: wifi.command(\"command\")"))) {
    return 0;
  }
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->runDirectCommand(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi direct command failed"));
  }
  return 1;
}

static numvar wifiPing(void) {
  if (!checkArgs(1, F("usage: wifi.ping(\"hostname\")"))) {
    return 0;
  }
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->ping(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi ping command failed"));
  }
  return 1;
}

static numvar wifiDNSLookup(void) {
  if (!checkArgs(1, F("usage: wifi.dnslookup(\"hostname\")"))) {
    return 0;
  }
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->dnsLookup(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi DNS lookup command failed"));
  }
  return 1;
}

static numvar wifiGetTime(void) {
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->printTime(Serial)) {
     speol(F("Error: Wi-Fi NTP time lookup command failed"));
  }
  return 1;
}

static numvar wifiSleep(void) {
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->goToSleep()) {
     speol(F("Error: Wi-Fi sleep command failed"));
  }
  return 1;
}

static numvar wifiWakeup(void) {
  if (!checkbp()) return 0;
  if (!WifiModule::instance.bp()->wakeUp()) {
     speol(F("Error: Wi-Fi wakeup command failed"));
  }
  return 1;
}

static numvar wifiVerbose(void) {
  // TODO
  return 1;
}

static numvar wifiStats(void) {
  sp(F("Number of associations to AP since boot: "));
  speol(WifiModule::instance.bp()->apConnCount);
  sp(F("Number of connections to HQ since last association: "));
  speol(WifiModule::instance.bp()->hqConnCount);
  sp(F("Seconds currently connected to HQ: "));
  if(WifiModule::instance.bp()->connectedAt) speol(SleepHandler::uptime().seconds - WifiModule::instance.bp()->connectedAt);
  else speol(0);
}

/****************************\
 *   MODULE CLASS STUFF     *
\****************************/

const __FlashStringHelper *WifiModule::name() const {
  return F("wifi");
}

bool WifiModule::enable() {
  if (!_enable())
    return false;

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

  static auto toggleBackpackVccCallback = build_callback(onToggleBackpackVcc);
  Scout.toggleBackpackVccCallbacks.append(toggleBackpackVccCallback);

  return true;
}

bool WifiModule::_enable() {
  for (uint8_t i = 0; i < Backpacks::num_backpacks; ++i) {
    if (Backpacks::info[i].id.model == 0x0001) {
      _bp = new WifiBackpack();
      if (!_bp)
        return false;
      if (!_bp->setup(&Backpacks::info[i]) || !_bp->associate()) {
        _disable();
        return false;
      }

      return true;
    }
  }

  speol("No wifi backpack found");
  return false;
}

void WifiModule::_disable() {
  delete _bp;
  _bp = NULL;
}

void WifiModule::loop() {
  if (_bp)
    _bp->loop();

  if (_bp->gs.unrecoverableError) {
    if (Scout.handler.isVerbose)
    {
      Serial.println(F("Unrecoverable error in the wifi backpack, rebooting scout...."));
      Serial.flush();
    }
    Scout.reboot();
    /* this doesn't appear to be working, somehow memory is corrupted and/or things seem to have gone badly, which may be the real problem :)
    Scout.disableBackpackVcc();
    delay(100);
    Scout.enableBackpackVcc();
    if (!_bp) {
      if (Scout.handler.isVerbose)
      {
        Serial.println(F("Failed to initialize after power cycle. Rebooting scout..."));
        Serial.flush();
      }
      Scout.reboot();
    }
    */
  }
}

void WifiModule::onToggleBackpackVcc(bool on) {
  if (!on && instance._bp) {
    instance._disable();
  } else if (on && instance.enabled()) {
    // Re-initialize
    instance._enable();
  }
}
