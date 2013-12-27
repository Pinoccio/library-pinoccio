#include <Arduino.h>
#include <utility/WiFiBackpack.h>

#define P_BACKPACK_WIFI_DEBUG
#ifdef P_BACKPACK_WIFI_DEBUG
#  define WD(x) x
#else
#  define WD(x)
#endif

WiFiBackpack::WiFiBackpack() { }

WiFiBackpack::~WiFiBackpack() { }

bool WiFiBackpack::setup() {
  Backpack::setup();

  if (!Gainspan.setup()) {
     D(Serial.println("FAIL: Setup failed"));
     return 0;
  }
}

bool WiFiBackpack::init() {

  if (!Gainspan.init()) {
    WD(Serial.println("Error: no response from Wi-Fi backpack"));
    return 0;
  }

  return 1;
}

void WiFiBackpack::loop() {
  Backpack::loop();
  // TODO if AP or HQ connection is gone, reconnect here
}

bool WiFiBackpack::apConfig(const char *ssid, const char *passphrase) {
  Gainspan.autoConfigure(ssid, passphrase);
}

void WiFiBackpack::printAPs() {
  Gainspan.send_cmd_w_resp(CMD_LIST_SSIDS);
}

void WiFiBackpack::printProfiles() {
  Gainspan.send_cmd_w_resp(CMD_PROFILEGET);
}

void WiFiBackpack::printCurrentNetworkStatus() {
  Gainspan.send_cmd_w_resp(CMD_NET_STATUS);
}

bool WiFiBackpack::connectToAP() {
  return Gainspan.autoConnect();
}

bool WiFiBackpack::connectToHQ(IPAddress server, uint16_t port) {
  // TODO
  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected");

    // Send message over UDP socket to peer device
    client.println("Hello server!");
  }
  else {
    // if connection setup failed:
    Serial.println("failed");
  }
}

void WiFiBackpack::dnsLookup(const char *host) {
  // TODO
}

void WiFiBackpack::ping(const char *host) {
  // TODO
}

bool WiFiBackpack::runDirectCommand(const char *command) {
  return Gainspan.send_raw_cmd_w_resp(command);
}

bool WiFiBackpack::goToSleep() {
  // TODO
}

bool WiFiBackpack::wakeUp() {
  // TODO
}


/* commands for auto-config
AT+WWPA=coworking775
AT+WAUTO=0,"Reno Collective"
ATC1
AT&W0
AT&Y0
AT+WA="Reno Collective"

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