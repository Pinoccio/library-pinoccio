#include <Arduino.h>
#include <SPI.h>
#include "WiFiBackpack.h"
#include "../../ScoutHandler.h"
#include "../../hq/HqHandler.h"
#include "src/bitlash.h"

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

WiFiBackpack::WiFiBackpack() : client(gs) { }

WiFiBackpack::~WiFiBackpack() { }

void WiFiBackpack::onAssociate(void *data) {
  WiFiBackpack& wifi = *(WiFiBackpack*)data;

  // Do a timesync
  IPAddress ip = wifi.gs.dnsLookup(NTP_SERVER);
  // TODO: Until https://github.com/arduino/Arduino/pull/1798 is merged,
  // we have to remove the constness here.
  if (ip == const_cast<IPAddress&>(INADDR_NONE) ||
      !wifi.gs.timeSync(ip, NTP_INTERVAL)) {
    Serial.println("Time sync failed");
  }
  
  wifi.apConnCount++;
}

void WiFiBackpack::onNcmConnect(void *data, GSCore::cid_t cid) {
  WiFiBackpack& wifi = *(WiFiBackpack*)data;

  wifi.client = cid;

  if (HqHandler::cacert_len != 0) {
    if (!wifi.client.enableTls(CA_CERTNAME_HQ)) {
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

void WiFiBackpack::onNcmDisconnect(void *data) {
  WiFiBackpack& wifi = *(WiFiBackpack*)data;

  wifi.client = GSCore::INVALID_CID;
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
  gs.onNcmConnect = onNcmConnect;
  gs.onNcmDisconnect = onNcmDisconnect;
  gs.eventData = this;

  if (!gs.begin(7))
    return false;

  if (HqHandler::cacert_len)
    gs.addCert(CA_CERTNAME_HQ, /* to_flash */ false, HqHandler::cacert, HqHandler::cacert_len);

  return true;
}

void WiFiBackpack::loop() {
  Backpack::loop();
  gs.loop();
  client = gs.getNcmCid();
}

bool WiFiBackpack::wifiConfig(const char *ssid, const char *passphrase, GSModule::GSSecurity security) {
  bool ok = true;
  ok = ok && gs.setSecurity(security);
  if (passphrase && *passphrase) {
    switch(security) {
      case GSModule::GS_SECURITY_WEP:
        ok = ok && gs.setWepPassphrase(passphrase);
      default:
        ok = ok && gs.setWpaPassphrase(passphrase);
    }
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

bool WiFiBackpack::autoConnectHq() {
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

bool WiFiBackpack::isHQConnected() {
  return client.connected() && client.sslConnected();
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
