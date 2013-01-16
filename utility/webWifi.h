#ifndef _PINOCCIO_WEB_WIFI_H_
#define _PINOCCIO_WEB_WIFI_H_

#include <avr/pgmspace.h>
#include <WString.h>

#include "webWifiClient.h"
#include "webWifiServer.h"
#include "halRgbLed.h"

#define MAX_SOCK_NUM 4

#define NORMAL_MODE     0
#define ADHOC_MODE      1
#define AP_MODE         2

// TODO : change these values
const uint8_t PROTO_TCP = 6;
const uint8_t PROTO_UDP = 7;

typedef struct _WIFI_PROFILE {
  String ssid;
  String security_key;
  String ip;
  String subnet;
  String gateway;
} WIFI_PROFILE;

class webWifi {
  public:
    static uint16_t _server_port[MAX_SOCK_NUM];

    void begin(WIFI_PROFILE*);
    void begin(WIFI_PROFILE*, uint8_t mode);

    void process();
    uint8_t connected();
    uint8_t socketOpen(String url, String port);

    void sendDeviceID();
    void sendResponse(String data);

    friend class WifiClient;
    friend class WifiServer;
};

extern webWifi Wifi;

#endif // _PINOCCIO_WEB_WIFI_H_