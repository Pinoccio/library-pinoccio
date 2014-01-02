#include <Arduino.h>
#include <utility/WiFiBackpack.h>

//#define P_BACKPACK_WIFI_DEBUG
#ifdef P_BACKPACK_WIFI_DEBUG
#  define WD(x) x
#else
#  define WD(x)
#endif

WiFiBackpack::WiFiBackpack() { }

WiFiBackpack::~WiFiBackpack() { }

bool WiFiBackpack::setup() {
  Serial.println(" WiFiBackpack::setup");
  Backpack::setup();

  if (!Gainspan.setup()) {
     WD(Serial.println("FAIL: Setup failed"));
     Serial.println("FAIL: Setup failed");
     return 0;
  }
  return 1; 
}

bool WiFiBackpack::init() {

  Serial.println(" WiFiBackpack::init");
  if (!Gainspan.init()) {
    //WD(Serial.println("Error: no response from Wi-Fi backpack"));

    Serial.println("Error: no response from Wi-Fi backpack");
    return 0;
  }
  client.autoConnect();

  return 1;
}

void WiFiBackpack::loop() {
  Backpack::loop();
  Gainspan.process();
}

bool WiFiBackpack::apConfig(const char *ssid, const char *passphrase, String host, String port) {
  String ip;
  if (client.connected()) {
    ip = Gainspan.dnsLookup(host);
  } else {
    ip = host;
  }

  Gainspan.autoConfigure(ssid, passphrase, ip, port);
}

bool WiFiBackpack::apConnect() {
  Gainspan.autoConnect();
}

void WiFiBackpack::printAPs() {
  Gainspan.send_cmd_w_resp(CMD_LIST_SSIDS);
}

void WiFiBackpack::printProfiles() {
  Gainspan.send_cmd_w_resp(CMD_PROFILEGET);
}

void WiFiBackpack::printCurrentNetworkStatus() {
  Gainspan.send_cmd_w_resp(CMD_NET_STATUS);
  Gainspan.send_cmd_w_resp(CMD_CURCID);
}

bool WiFiBackpack::dnsLookup(const char *host) {
  Serial.println(Gainspan.dnsLookup(host));
}

bool WiFiBackpack::ping(const char *host) {
  Gainspan.ping(host);
}

bool WiFiBackpack::runDirectCommand(const char *command) {
  Gainspan.send_raw_cmd_w_resp(command);
}

bool WiFiBackpack::goToSleep() {
  Gainspan.send_cmd(CMD_PSDPSLEEP);
}

bool WiFiBackpack::wakeUp() {
  Gainspan.send_cmd_w_resp(CMD_AT);
}

bool WiFiBackpack::getTime() {
  Gainspan.send_cmd_w_resp(CMD_GETTIME);
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
