#include <Pinoccio.h>
#include "webWifi.h"
#include "webGainspan.h"
#include "halRgbLed.h"

uint16_t webWifi::_server_port[MAX_SOCK_NUM] = { 0, 0, 0, 0 };

void webWifi::begin() 
{
  WIFI_PROFILE profile = {
          /* SSID */ "",
          /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
          /* subnet mask */ "",
          /* Gateway IP */ "" };
  
}

void webWifi::begin(WIFI_PROFILE* w_prof)
{
  begin(w_prof, NORMAL_MODE);
}

void webWifi::begin(WIFI_PROFILE* w_prof, uint8_t mode)
{
  D(Serial.println("DEBUG: Wifi::begin 1"));
  // setup LEDs
  RgbLed.turnOff();
  D(Serial.println("DEBUG: Wifi::begin 2"));
  Gainspan.mode = mode;
  D(Serial.println("DEBUG: Wifi::begin 3"));
  // initialize device
  if (!Gainspan.init()) {
		RgbLed.red();
    D(Serial.println("DEBUG: Wifi::begin 3.1"));
    return;
  }
  D(Serial.println("DEBUG: Wifi::begin 4"));
  // configure params
  Gainspan.configure((GS_PROFILE*)w_prof);
  D(Serial.println("DEBUG: Wifi::begin 5"));
  // initiate wireless connection
  while (!Gainspan.connect());
  D(Serial.println("DEBUG: Wifi::begin 6"));
	RgbLed.green();
  D(Serial.println("DEBUG: Wifi::begin 7"));
}

void webWifi::process()
{
  Gainspan.process();
}

uint8_t webWifi::socketOpen(String url, String port)
{
#if 0
  String ip;

  // get IP address from URL
  if ((ip = Gainspan.dns_lookup(url)) == "0.0.0.0") {
    return 0;
  }

  // open socket connection
  if (!Gainspan.connect_socket("192.168.0.100", "32000")) {
    return 0;
  }
#endif
  return 1;
}

webWifi Wifi;