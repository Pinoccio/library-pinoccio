/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Scout.h>
#include <SPI.h>
#include "Wifi.h"
#include "../../backpacks/Backpacks.h"
#include "../../ScoutHandler.h"
#include "src/bitlash.h"
extern "C" {
#include "key/key.h"
}

using pinoccio::WifiModule;

WifiModule WifiModule::instance;

static void print_line(const uint8_t *buf, uint16_t len, void *data) {
  while (len--)
    spb(*buf++);
  speol();
}

static StringBuffer wifiReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[%s,%s]]",
          keyMap("wifi", 0),
          keyMap("connected", 0),
          keyMap("hq", 0),
          WifiModule::instance.isAPConnected() ? "true" : "false",
          Scout.handler.client->connected() ? "true" : "false");
  return Scout.handler.report(report);
}

numvar wifiReport(void) {
  speol(wifiReportHQ());
  return 1;
}

numvar wifiStatus(void) {
  if (getarg(0) > 0 && getarg(1) == 1) {
    WifiModule::instance.printProfiles(Serial);
  } else {
    WifiModule::instance.printFirmwareVersions(Serial);
    WifiModule::instance.printCurrentNetworkStatus(Serial);
  }
  return 1;
}

static numvar wifiList(void) {
  if (!WifiModule::instance.printAPs(Serial)) {
    speol(F("Error: Scan failed"));
    return 0;
  }
  return 1;
}

static numvar wifixConfig(void) {
  if (!checkArgs(1, 2, F("usage: wifi.config(\"wifiAPName\", \"wifiAPPassword\")"))) {
    return 0;
  }

  if (!WifiModule::instance.wifiConfig((const char *)getstringarg(1), (const char *)getstringarg(2))) {
    speol(F("Error: saving WifiModule::instance.configuration data failed"));
  }
  return 1;
}

static numvar wifixDhcp(void) {
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!WifiModule::instance.wifiDhcp(host)) {
    speol(F("Error: saving WifiModule::instance.configuration data failed"));
  }
  return 1;
}

static numvar wifixStatic(void) {
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

  if (!WifiModule::instance.wifiStatic(ip, nm, gw, dns)) {
    speol(F("Error: saving WifiModule::instance.configuration data failed"));
    return 0;
  }
  return 1;
}

static numvar wifiDisassociate(void) {
  WifiModule::instance.disassociate();
  return 1;
}

static numvar wifiReassociate(void) {
  return WifiModule::instance.reassociate();
}

static numvar wifiHQ(void) {
  if (!checkArgs(1, 2, F("usage: wifi.hq(\"host\" [,port])"))) {
    return 0;
  }
  char *host = (char*)getstringarg(1);
  free(WifiModule::instance.hq_host);
  WifiModule::instance.hq_host = strdup(host);
  if(getarg(0) > 1)
  {
    WifiModule::instance.hq_port = getarg(2);
  }
  WifiModule::instance.reassociate();
  return 1;
}

static numvar wifiCommand(void) {
  if (!checkArgs(1, F("usage: wifi.command(\"command\")"))) {
    return 0;
  }
  if (!WifiModule::instance.runDirectCommand(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi direct command failed"));
  }
  return 1;
}

static numvar wifiPing(void) {
  if (!checkArgs(1, F("usage: wifi.ping(\"hostname\")"))) {
    return 0;
  }
  if (!WifiModule::instance.ping(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi ping command failed"));
  }
  return 1;
}

static numvar wifiDNSLookup(void) {
  if (!checkArgs(1, F("usage: wifi.dnslookup(\"hostname\")"))) {
    return 0;
  }
  if (!WifiModule::instance.dnsLookup(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi DNS lookup command failed"));
  }
  return 1;
}

static numvar wifiGetTime(void) {
  if (!WifiModule::instance.printTime(Serial)) {
     speol(F("Error: Wi-Fi NTP time lookup command failed"));
  }
  return 1;
}

static numvar wifiSleep(void) {
  if (!WifiModule::instance.goToSleep()) {
     speol(F("Error: Wi-Fi sleep command failed"));
  }
  return 1;
}

static numvar wifiWakeup(void) {
  if (!WifiModule::instance.wakeUp()) {
     speol(F("Error: Wi-Fi wakeup command failed"));
  }
  return 1;
}

static numvar wifiVerbose(void) {
  if(getarg(0) == 1 && getarg(1) == 0)
  {
    WifiModule::instance.verbose = false;
  }else{
    WifiModule::instance.verbose = true;
  }
  return WifiModule::instance.verbose?1:0;
}

static numvar wifiStats(void) {
  sp(F("Number of connections to AP since boot: "));
  speol(WifiModule::instance.apConnCount);
  sp(F("Number of connections to HQ since boot: "));
  speol(WifiModule::instance.hqConnCount);
}

/* commands for auto-config
AT+WWPA=password
AT+WAUTO=0,"SSID"
ATC1
AT&W0
AT&Y0
AT+WA="SSID"

AT+NCTCP=192.168.1.83,80
AT+STORENWCONN

// run after wake up from sleep
AT+RESTORENWCONN

// list AP stats
AT+NSTAT=?

// list current connections
AT+CID=?

// list config profiles
AT&V

// enter deep sleep
AT+PSDPSLEEP
*/

void WifiModule::onAssociate(void *data) {

  if(WifiModule::instance.verbose) Serial.println("associated, connecting to hq");
  WifiModule::instance.apConnCount++;
  // TODO, update GSTcpClient to support hostnames
  IPAddress ip;
  WifiModule::instance.gs.parseIpAddress(&ip,WifiModule::instance.hq_host);
  if(Scout.handler.client->connect(ip, WifiModule::instance.hq_port))
  {
    if(WifiModule::instance.verbose) Serial.println("connected to hq");
    WifiModule::instance.hqConnCount++;
    leadHQConnect();
  }else{
    if(WifiModule::instance.verbose) Serial.println("connection failed");
  }
  
}

const __FlashStringHelper *WifiModule::name() const {
  return F("wifi");
}

bool WifiModule::load() {
  // scripting is fun
  Shell.addFunction("wifi.report", wifiReport);
  Shell.addFunction("wifi.hq", wifiHQ);
  Shell.addFunction("wifi.status", wifiStatus);
  Shell.addFunction("wifi.list", wifiList);
  Shell.addFunction("wifi.config", wifixConfig);
  Shell.addFunction("wifi.dhcp", wifixDhcp);
  Shell.addFunction("wifi.static", wifixStatic);
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

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  gs.onAssociate = onAssociate;
  gs.eventData = this;

  if(!hq_host)
  {
//    hq_host = strdup("pool.base.pinocc.io");
    hq_host = strdup("23.239.2.207");
    hq_port = 22756;
  }
  Scout.handler.client = new GSTcpClient(gs);

  if (getHardwareMajorRevision() == 1 && getHardwareMinorRevision() == 1) {
    if (!gs.begin(7, 5)) return false;
  } else {
    if (!gs.begin(7)) return false;
  }

  // When association fails, keep retrying indefinately (at least it
  // seems that a retry count of 0 means that, even though the
  // documentation says it should be >= 1).
  gs.setNcmParam(GSModule::GS_NCM_L3_CONNECT_RETRY_COUNT, 0);
  gs.setNcm(/* enable */ true, /* associate_only */ true, /* remember */ false);
  return true;
}

// 5 minutes until check/reconnect
#define DOWN_TIMEOUT (60*5)+5
uint32_t down_check = 0;
void WifiModule::loop() {
  gs.loop();

  uint32_t now = Scout.uptime();
  // only validate/reset when no hq is active and not too fast
  if(now - Scout.handler.active > DOWN_TIMEOUT && now - down_check > DOWN_TIMEOUT)
  {
    if(verbose)
    {
      Serial.print("wifi validation check at ");
      Serial.print(now);
      Serial.print(" hq ");
      Serial.print(Scout.handler.active);
      Serial.print(" check ");
      Serial.println(down_check);
    }
    down_check = now;
    // check if gainspan is still responding
    if(!gs.writeCommandCheckOk("AT"))
    {
      Serial.println("gainspan timed out, restarting");
      Scout.reboot();
    }
    // just reassociate to try reconnecting clean
    reassociate();
  }
}

static bool isWepKey(const char *key) {
  int len = 0;
  while (key[len] && len <= 26) {
    if (!isxdigit(key[len]))
      return false;
    ++len;
  }

  return len == 10 || len == 26;
}

bool WifiModule::wifiConfig(const char *ssid, const char *passphrase) {
  bool ok = true;
  ok = ok && gs.setSecurity(GSModule::GS_SECURITY_AUTO);
  if (passphrase && *passphrase) {
    // Setting WEP passphrase will return error if phrase isn't exactly
    // 10 or 26 hex bytes
    if (isWepKey(passphrase))
      ok = ok && gs.setWepPassphrase(passphrase);
    ok = ok && gs.setWpaPassphrase(passphrase);
  }
  ok = ok && gs.setAutoAssociate(ssid);
  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  // Ignore setDefaultProfile failure, since it fails also when only a
  // single profile is available
  ok && gs.setDefaultProfile(0);
  return ok;
}

bool WifiModule::wifiDhcp(const char *hostname) {
  bool ok = true;
  ok = ok && gs.setDhcp(true, hostname);
  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  // Ignore setDefaultProfile failure, since it fails also when only a
  // single profile is available
  ok && gs.setDefaultProfile(0);
  return ok;
}

bool WifiModule::wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns) {
  bool ok = true;
  ok = ok && gs.setDhcp(false);
  ok = ok && gs.setStaticIp(ip, netmask, gw);
  ok = ok && gs.setDns(dns);

  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  // Ignore setDefaultProfile failure, since it fails also when only a
  // single profile is available
  ok && gs.setDefaultProfile(0);
  return ok;
}

bool WifiModule::reassociate() {
  if(verbose) Serial.println("reassociating");
  disassociate();

  return gs.setNcm(/* enable */ true, /* associate_only */ true, /* remember */ false);
}

void WifiModule::disassociate() {
  if(!isAPConnected()) return;
  // this delay is important--The Gainspan module with 2.5.1 firmware
  // will hang if the NCM disassociate is called too soon after boot.
  if (millis() < 5000) {
    delay(4000);
  }
  gs.setNcm(false);
  gs.disassociate();
}

bool WifiModule::printAPs(Print& p) {
  // this delay is important--The Gainspan module with 2.5.1 firmware
  // will hang if AT+WS is called too soon after boot.
  if (millis() < 5000) {
    delay(4000);
  }
  return runDirectCommand(p, "AT+WS");
}

void WifiModule::printProfiles(Print& p) {
  runDirectCommand(p, "AT&V");
}

void WifiModule::printCurrentNetworkStatus(Print& p) {
  runDirectCommand(p, "AT+NSTAT=?");
  runDirectCommand(p, "AT+CID=?");
}

void WifiModule::printFirmwareVersions(Print& p) {
  runDirectCommand(p, "AT+VER=?");
}

int WifiModule::getHardwareMajorRevision() {
  Pbbe::UniqueId &id = Backpacks::info[0].id;
  Pbbe::MajorMinor rev = Pbbe::extractMajorMinor(id.revision);
  return rev.major;
}

int WifiModule::getHardwareMinorRevision() {
  Pbbe::UniqueId &id = Backpacks::info[0].id;
  Pbbe::MajorMinor rev = Pbbe::extractMajorMinor(id.revision);
  return rev.minor;
}
 
bool WifiModule::isAPConnected() {
  return gs.isAssociated();
}

bool WifiModule::dnsLookup(Print& p, const char *host) {
  // TODO
  return false;
}

bool WifiModule::ping(Print &p, const char *host) {
  // TODO
  return false;
}

bool WifiModule::runDirectCommand(Print &p, const char *command) {
  gs.writeCommand("%s", command);
  return (gs.readResponse(print_line, NULL) == GSCore::GS_SUCCESS);
}

bool WifiModule::goToSleep() {
  // TODO
  //Gainspan.send_cmd(CMD_PSDPSLEEP);
  return false;
}

bool WifiModule::wakeUp() {
  // TODO
  // Gainspan.send_cmd_w_resp(CMD_AT);
  return false;
}

bool WifiModule::printTime(Print &p) {
  return runDirectCommand(p, "AT+GETTIME=?");
}
