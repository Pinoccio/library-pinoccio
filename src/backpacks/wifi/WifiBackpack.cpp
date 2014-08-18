/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include "WifiBackpack.h"
#include "../../Scout.h"
#include "../../ScoutHandler.h"
#include "../../hq/HqHandler.h"
#include "../../SleepHandler.h"
#include "src/bitlash.h"

using namespace pinoccio;

// Be careful with using non-alphanumerics like '-' here, they might
// silently cause SSL to fail
#define CA_CERTNAME_HQ "hq.ca"
#define NTP_SERVER "pool.ntp.org"
// Sync on connect and every 24 hours thereafter
#define NTP_INTERVAL (3600L * 24)

static void print_line(const uint8_t *buf, uint16_t len, void *data) {
  while (len--)
    spb(*buf++);
  speol();
}

WifiBackpack::WifiBackpack() : client(gs) { }

WifiBackpack::~WifiBackpack() {
  gs.end();
}

void WifiBackpack::onAssociate(void *data) {
  WifiBackpack& wifi = *(WifiBackpack*)data;
  wifi.apConnCount++;

  if (HqHandler::use_tls()) {
    // Do a timesync
    IPAddress ip = wifi.gs.dnsLookup(NTP_SERVER);
    if (ip == INADDR_NONE ||
        !wifi.gs.timeSync(ip, NTP_INTERVAL)) {
      if (Scout.handler.isVerbose)
      {
        Serial.println(F("Time sync failed, reassociating to retry"));
      }
      wifi.associate();
    }
  }
  // connect to HQ now
  wifi.connectToHq();
}


bool WifiBackpack::connectToHq() {
  IPAddress ip;
  if (!gs.parseIpAddress(&ip, HqHandler::host().c_str())) {
    ip = gs.dnsLookup(HqHandler::host().c_str());

    if (ip == INADDR_NONE) {
      if (Scout.handler.isVerbose)
      {
        Serial.print(F("Failed to resolve "));
        Serial.print(HqHandler::host());
        Serial.println(F(", reassociating to retry"));
      }
      return false;
    }
  }

  if (!client.connect(ip, HqHandler::port())) {
    if (Scout.handler.isVerbose)
    {
      Serial.println(F("HQ connection failed, reassociating to retry"));
    }
    associate();
    return false;
  }

  if (HqHandler::use_tls()) {
    if (!client.enableTls(CA_CERTNAME_HQ)) {
      // Failed SSL negotiation kills the connection
      if (Scout.handler.isVerbose)
      {
        Serial.println(F("SSL negotiation to HQ failed, reassociating to retry"));
      }
      associate();
      return false;
    }
  }

  // TODO: Don't call leadHQConnect directly?
  leadHQConnect();
  hqConnCount++;
  return true;
}

bool WifiBackpack::setup(BackpackInfo *info) {
  // Alternatively, use the UART for Wifi backpacks that still have the
  // UART firmware running on them
  // Serial1.begin(115200);
  // return gs.begin(Serial1);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  gs.onAssociate = onAssociate;
  gs.eventData = this;

  if (info->id.revision == 0x11) {
    if (!gs.begin(7, 5)) {
      return false;
    }
  } else {
    if (!gs.begin(7)) {
      return false;
    }
  }
  if (HqHandler::cacert_len)
    gs.addCert(CA_CERTNAME_HQ, /* to_flash */ false, HqHandler::cacert, HqHandler::cacert_len);

  return true;
}

void WifiBackpack::loop() {
  gs.loop();
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

bool WifiBackpack::wifiConfig(const char *ssid, const char *passphrase) {
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
  associate();
  return ok;
}

bool WifiBackpack::wifiDhcp(const char *hostname) {
  bool ok = true;
  ok = ok && gs.setDhcp(true, hostname);
  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  // Ignore setDefaultProfile failure, since it fails also when only a
  // single profile is available
  ok && gs.setDefaultProfile(0);
  associate();
  return ok;
}

bool WifiBackpack::wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns) {
  bool ok = true;
  ok = ok && gs.setDhcp(false);
  ok = ok && gs.setStaticIp(ip, netmask, gw);
  ok = ok && gs.setDns(dns);

  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  // Ignore setDefaultProfile failure, since it fails also when only a
  // single profile is available
  ok && gs.setDefaultProfile(0);
  associate();
  return ok;
}

bool WifiBackpack::associate() {
  // Try to disable the NCM in case it's already running
  disassociate();
  associating = true;

  // When association fails, keep retrying indefinately (at least it
  // seems that a retry count of 0 means that, even though the
  // documentation says it should be >= 1).
  gs.setNcmParam(GSModule::GS_NCM_L3_CONNECT_RETRY_COUNT, 0);

  return gs.setNcm(/* enable */ true, /* associate_only */ true, /* remember */ false);
}

void WifiBackpack::disassociate() {
  // nothing to disassociate if not associating
  if(!associating) return;

  // this delay is important--The Gainspan module with 2.5.1 firmware
  // will hang if the NCM disassociate is called too soon after boot.
  if (millis() < 5000) {
    delay(4000);
  }
  gs.setNcm(false);
  gs.disassociate();
}

bool WifiBackpack::printAPs(Print& p) {
  // this delay is important--The Gainspan module with 2.5.1 firmware
  // will hang if AT+WS is called too soon after boot.
  if (millis() < 5000) {
    delay(4000);
  }
  return runDirectCommand(p, "AT+WS");
}

void WifiBackpack::printProfiles(Print& p) {
  runDirectCommand(p, "AT&V");
}

void WifiBackpack::printCurrentNetworkStatus(Print& p) {
  runDirectCommand(p, "AT+NSTAT=?");
  runDirectCommand(p, "AT+CID=?");
}

void WifiBackpack::printFirmwareVersions(Print& p) {
  runDirectCommand(p, "AT+VER=?");
}

bool WifiBackpack::isAPConnected() {
  return gs.isAssociated();
}

bool WifiBackpack::isHQConnected() {
  return client.connected() && (!HqHandler::use_tls() || client.sslConnected());
}

bool WifiBackpack::dnsLookup(Print& p, const char *host) {
  // TODO
  return false;
}

bool WifiBackpack::ping(Print &p, const char *host) {
  // TODO
  return false;
}

bool WifiBackpack::runDirectCommand(Print &p, const char *command) {
  gs.writeCommand("%s", command);
  return (gs.readResponse(print_line, NULL) == GSCore::GS_SUCCESS);
}

bool WifiBackpack::goToSleep() {
  // TODO
  //Gainspan.send_cmd(CMD_PSDPSLEEP);
  return false;
}

bool WifiBackpack::wakeUp() {
  // TODO
  // Gainspan.send_cmd_w_resp(CMD_AT);
  return false;
}

bool WifiBackpack::printTime(Print &p) {
  return runDirectCommand(p, "AT+GETTIME=?");
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
