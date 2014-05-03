#include <Arduino.h>
#include <SPI.h>
#include "WiFiBackpack.h"
#include "../../ScoutHandler.h"
#include "../../hq/HqHandler.h"
#include "src/bitlash.h"

static void print_line(const uint8_t *buf, uint16_t len, void *data) {
  while (len--)
    spb(*buf++);
  speol();
}

WiFiBackpack::WiFiBackpack() : server(gs) { }

WiFiBackpack::~WiFiBackpack() { }

void WiFiBackpack::onAssociate(void *data) {
  WiFiBackpack& wifi = *(WiFiBackpack*)data;

  wifi.apConnCount++;
  if(wifi.onOnline) wifi.onOnline();

}

void WiFiBackpack::onDisassociate(void *data) {
  WiFiBackpack& wifi = *(WiFiBackpack*)data;

  if(wifi.onOffline) wifi.onOffline();
}

bool WiFiBackpack::setup() {
  Backpack::setup();

  // Alternatively, use the UART for Wifi backpacks that still have the
  // UART firmware running on them
  // Serial1.begin(115200);
  // return gs.begin(Serial1);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  gs.onAssociate = onAssociate;
  gs.onDisassociate = onDisassociate;
  gs.eventData = this;

  if (!gs.begin(7))
    return false;

  return true;
}

void WiFiBackpack::loop() {
  Backpack::loop();
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

bool WiFiBackpack::wifiConfig(const char *ssid, const char *passphrase) {
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

bool WiFiBackpack::wifiDhcp(const char *hostname) {
  bool ok = true;
  ok = ok && gs.setDhcp(true, hostname);
  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  // Ignore setDefaultProfile failure, since it fails also when only a
  // single profile is available
  ok && gs.setDefaultProfile(0);
  return ok;
}

bool WiFiBackpack::wifiStatic(IPAddress ip, IPAddress netmask, IPAddress gw, IPAddress dns) {
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

bool WiFiBackpack::autoOnline() {
  // Try to disable the NCM in case it's already running
  gs.setNcm(false);

  // When association fails, keep retrying indefinately (at least it
  // seems that a retry count of 0 means that, even though the
  // documentation says it should be >= 1).
  gs.setNcmParam(GSModule::GS_NCM_L3_CONNECT_RETRY_COUNT, 0);

  return gs.setNcm(/* enable */ true, /* associate_only */ true, /* remember */ false);
}

void WiFiBackpack::disassociate() {
  // this delay is important--The Gainspan module with 2.5.1 firmware
  // will hang if the NCM disassociate is called too soon after boot.
  if (millis() < 5000) {
    delay(4000);
  }
  gs.setNcm(false);
  gs.disassociate();
}

bool WiFiBackpack::printAPs(Print& p) {
  // this delay is important--The Gainspan module with 2.5.1 firmware
  // will hang if AT+WS is called too soon after boot.
  if (millis() < 5000) {
    delay(4000);
  }
  return runDirectCommand(p, "AT+WS");
}

void WiFiBackpack::printProfiles(Print& p) {
  runDirectCommand(p, "AT&V");
}

void WiFiBackpack::printCurrentNetworkStatus(Print& p) {
  runDirectCommand(p, "AT+NSTAT=?");
  runDirectCommand(p, "AT+CID=?");
}

void WiFiBackpack::printFirmwareVersions(Print& p) {
  runDirectCommand(p, "AT+VER=?");
}

bool WiFiBackpack::isAPConnected() {
  return gs.isAssociated();
}

bool WiFiBackpack::dnsLookup(Print& p, const char *host) {
  // TODO
  return false;
}

bool WiFiBackpack::ping(Print &p, const char *host) {
  // TODO
  return false;
}

bool WiFiBackpack::runDirectCommand(Print &p, const char *command) {
  gs.writeCommand("%s", command);
  return (gs.readResponse(print_line, NULL) == GSCore::GS_SUCCESS);
}

bool WiFiBackpack::goToSleep() {
  // TODO
  //Gainspan.send_cmd(CMD_PSDPSLEEP);
  return false;
}

bool WiFiBackpack::wakeUp() {
  // TODO
  // Gainspan.send_cmd_w_resp(CMD_AT);
  return false;
}

bool WiFiBackpack::printTime(Print &p) {
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
