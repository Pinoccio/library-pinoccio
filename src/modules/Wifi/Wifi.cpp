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
#include "../../ScoutHandler.h"
#include "../../hq/HqHandler.h"
#include "src/bitlash.h"
extern "C" {
#include "key/key.h"
}

// Be careful with using non-alphanumerics like '-' here, they might
// silently cause SSL to fail
#define CA_CERTNAME_HQ "hq.ca"
#define NTP_SERVER "pool.ntp.org"
// Sync on connect and every 24 hours thereafter
#define NTP_INTERVAL (3600L * 24)

static numvar wifiReport(void);
static numvar wifiStatus(void);
static numvar wifiList(void);
static numvar wifixConfig(void);
static numvar wifixDhcp(void);
static numvar wifixStatic(void);
static numvar wifiReassociate(void);
static numvar wifiDisassociate(void);
static numvar wifiCommand(void);
static numvar wifiPing(void);
static numvar wifiDNSLookup(void);
static numvar wifiGetTime(void);
static numvar wifiSleep(void);
static numvar wifiWakeup(void);
static numvar wifiVerbose(void);
static numvar wifiStats(void);

static void print_line(const uint8_t *buf, uint16_t len, void *data) {
  while (len--)
    spb(*buf++);
  speol();
}

const char *WifiModule::name() {
  return "wifi";
}

void WifiModule::onAssociate(void *data) {
  WifiModule& wifi = *(WifiModule*)data;

  #ifdef USE_TLS
  // Do a timesync
  IPAddress ip = wifi.gs.dnsLookup(NTP_SERVER);
  if (ip == INADDR_NONE ||
      !wifi.gs.timeSync(ip, NTP_INTERVAL)) {
    Serial.println("Time sync failed, reassociating to retry");
    wifi.autoConnectHq();
  }
  #endif
  
  wifi.apConnCount++;
}

void WifiModule::onNcmConnect(void *data, GSCore::cid_t cid) {
  WifiModule& wifi = *(WifiModule*)data;

  *(Scout.handler.client) = cid;

  if (HqHandler::cacert_len != 0) {
    if (!Scout.handler.client->enableTls(CA_CERTNAME_HQ)) {
      // If enableTls fails, the NCM doesn't retry the TCP
      // connection. We restart the entire association to get NCM to
      // retry the TCP connection instead.
      Serial.println("SSL negotiation to HQ failed, reassociating to retry");
      wifi.autoConnectHq();
      return;
    }
  }
  
  wifi.hqConnCount++;

  // TODO: Don't call leadHQConnect directly?
  leadHQConnect();
}

void WifiModule::onNcmDisconnect(void *data) {
  WifiModule& wifi = *(WifiModule*)data;

  *(Scout.handler.client) = GSCore::INVALID_CID;
}

// this is our singleton object for c function pointer convenience
WifiModule *wifi;
void WifiModule::setup() {

  wifi = this;
  addBitlashFunction("wifi.report", (bitlash_function) wifiReport);
  addBitlashFunction("wifi.status", (bitlash_function) wifiStatus);
  addBitlashFunction("wifi.list", (bitlash_function) wifiList);
  addBitlashFunction("wifi.config", (bitlash_function) wifixConfig);
  addBitlashFunction("wifi.dhcp", (bitlash_function) wifixDhcp);
  addBitlashFunction("wifi.static", (bitlash_function) wifixStatic);
  addBitlashFunction("wifi.reassociate", (bitlash_function) wifiReassociate);
  addBitlashFunction("wifi.disassociate", (bitlash_function) wifiDisassociate);
  addBitlashFunction("wifi.command", (bitlash_function) wifiCommand);
  addBitlashFunction("wifi.ping", (bitlash_function) wifiPing);
  addBitlashFunction("wifi.dnslookup", (bitlash_function) wifiDNSLookup);
  addBitlashFunction("wifi.gettime", (bitlash_function) wifiGetTime);
  addBitlashFunction("wifi.sleep", (bitlash_function) wifiSleep);
  addBitlashFunction("wifi.wakeup", (bitlash_function) wifiWakeup);
  addBitlashFunction("wifi.verbose", (bitlash_function) wifiVerbose);
  addBitlashFunction("wifi.stats", (bitlash_function) wifiStats);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  gs.onAssociate = onAssociate;
  gs.onNcmConnect = onNcmConnect;
  gs.onNcmDisconnect = onNcmDisconnect;
  gs.eventData = this;
  Scout.handler.client = new GSTcpClient(gs);

  if (!gs.begin(7)) return;

  if (HqHandler::cacert_len)
    gs.addCert(CA_CERTNAME_HQ, /* to_flash */ false, HqHandler::cacert, HqHandler::cacert_len);

  autoConnectHq();
  
}

void WifiModule::loop() {
  gs.loop();
  *(Scout.handler.client) = gs.getNcmCid();
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

bool WifiModule::autoConnectHq() {
  // Try to disable the NCM in case it's already running
  gs.setNcm(false);

  // When association fails, keep retrying indefinately (at least it
  // seems that a retry count of 0 means that, even though the
  // documentation says it should be >= 1).
  gs.setNcmParam(GSModule::GS_NCM_L3_CONNECT_RETRY_COUNT, 0);

  // When association succeeds, but the TCP connection fails, keep
  // retrying to connect (but not as fast as the default 500ms between
  // attempts) indefinately (at least it seems that a retry count of 0
  // means that, documentation doesn't say).
  gs.setParam(GSModule::GS_PARAM_L4_RETRY_PERIOD, 1000 /* x 10ms */);
  gs.setParam(GSModule::GS_PARAM_L4_RETRY_COUNT, 0);

  return gs.setAutoConnectClient(HqHandler::host, HqHandler::port) &&
         gs.setNcm(/* enable */ true, /* associate_only */ false, /* remember */ false);
}

void WifiModule::disassociate() {
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

static StringBuffer wifiReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[%s,%s]]",
          keyMap("wifi", 0),
          keyMap("connected", 0),
          keyMap("hq", 0),
          wifi->isAPConnected() ? "true" : "false",
          Scout.handler.client->connected() ? "true" : "false");
  return Scout.handler.report(report);
}

numvar wifiReport(void) {
  speol(wifiReportHQ());
  return 1;
}

numvar wifiStatus(void) {
  if (getarg(0) > 0 && getarg(1) == 1) {
    wifi->printProfiles(Serial);
  } else {
    wifi->printFirmwareVersions(Serial);
    wifi->printCurrentNetworkStatus(Serial);
  }
  return 1;
}

static numvar wifiList(void) {
  if (!wifi->printAPs(Serial)) {
    speol(F("Error: Scan failed"));
    return 0;
  }
  return 1;
}

static numvar wifixConfig(void) {
  if (!checkArgs(2, F("usage: wifi.config(\"wifiAPName\", \"wifiAPPassword\")"))) {
    return 0;
  }

  if (!wifi->wifiConfig((const char *)getstringarg(1), (const char *)getstringarg(2))) {
    speol(F("Error: saving wifi->configuration data failed"));
  }
  return 1;
}

static numvar wifixDhcp(void) {
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!wifi->wifiDhcp(host)) {
    speol(F("Error: saving wifi->configuration data failed"));
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

  if (!wifi->wifiStatic(ip, nm, gw, dns)) {
    speol(F("Error: saving wifi->configuration data failed"));
    return 0;
  }
  return 1;
}

static numvar wifiDisassociate(void) {
  wifi->disassociate();
  return 1;
}

static numvar wifiReassociate(void) {
  // This restart the NCM
  return wifi->autoConnectHq();
}

static numvar wifiCommand(void) {
  if (!checkArgs(1, F("usage: wifi.command(\"command\")"))) {
    return 0;
  }
  if (!wifi->runDirectCommand(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi direct command failed"));
  }
  return 1;
}

static numvar wifiPing(void) {
  if (!checkArgs(1, F("usage: wifi.ping(\"hostname\")"))) {
    return 0;
  }
  if (!wifi->ping(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi ping command failed"));
  }
  return 1;
}

static numvar wifiDNSLookup(void) {
  if (!checkArgs(1, F("usage: wifi.dnslookup(\"hostname\")"))) {
    return 0;
  }
  if (!wifi->dnsLookup(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi DNS lookup command failed"));
  }
  return 1;
}

static numvar wifiGetTime(void) {
  if (!wifi->printTime(Serial)) {
     speol(F("Error: Wi-Fi NTP time lookup command failed"));
  }
  return 1;
}

static numvar wifiSleep(void) {
  if (!wifi->goToSleep()) {
     speol(F("Error: Wi-Fi sleep command failed"));
  }
  return 1;
}

static numvar wifiWakeup(void) {
  if (!wifi->wakeUp()) {
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
  speol(wifi->apConnCount);
  sp(F("Number of connections to HQ since boot: "));
  speol(wifi->hqConnCount);
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
