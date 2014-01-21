#include <Arduino.h>
#include <SPI.h>
#include <utility/WiFiBackpack.h>
#include "../ScoutHandler.h"
#include "../HqHandler.h"

#define CA_CERTNAME_HQ "hq-ca"

static void print_line(const uint8_t *buf, uint16_t len, void *data) {
  static_cast<Print*>(data)->write(buf, len);
  static_cast<Print*>(data)->println();
}

WiFiBackpack::WiFiBackpack() : client(gs) { }

WiFiBackpack::~WiFiBackpack() { }

bool WiFiBackpack::setup() {
  Backpack::setup();

  // Alternatively, use the UART for Wifi backpacks that still have the
  // UART firmware running on them
  // Serial1.begin(115200);
  // return gs.begin(Serial1);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);


  if (!gs.begin(7))
    return false;

  if (HqHandler::cacert_len)
    gs.addCert(CA_CERTNAME_HQ, /* to_flash */ false, HqHandler::cacert, HqHandler::cacert_len);
}

void WiFiBackpack::loop() {
  Backpack::loop();
  client = gs.getNcmCid();
  if (!client.connected()) {
    hqConnected = false;
  } else if (!hqConnected) {
    if (HqHandler::cacert_len == 0 || client.enableTls(CA_CERTNAME_HQ)) {
      leadHQConnect();
      hqConnected = true;
    }
  }
  // TODO: Don't call leadHQConnect directly
  // TODO: There is a race condition here: If a disconnect and connect
  // happen quickly before we can notice the disconnect, leadHqConnect
  // will not be called for the new connection.
}

bool WiFiBackpack::wifiConfig(const char *ssid, const char *passphrase) {
  bool ok = true;
  ok = ok && gs.setSecurity(GSModule::GS_SECURITY_AUTO);
  ok = ok && gs.setWpaPassphrase(passphrase);
  ok = ok && gs.setAutoAssociate(ssid);
  // Remember these settings through a reboot
  ok = ok && gs.saveProfile(0);
  ok = ok && gs.setDefaultProfile(0);
  return ok;
}

bool WiFiBackpack::autoConnectHq() {
  // Try to disable the NCM in case it's already running
  gs.setNcm(false);
  return gs.setAutoConnectClient(HqHandler::host, HqHandler::port) &&
         gs.setNcm(/* enable */ true, /* associate_only */ false, /* remember */ false);
}

void WiFiBackpack::printAPs(Print& p) {
  runDirectCommand(p, "AT+WS");
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
  return gs.readResponse(print_line, &p);
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
