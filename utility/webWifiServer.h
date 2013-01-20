#ifndef _PINOCCIO_WEB_WIFI_SERVER_H_
#define _PINOCCIO_WEB_WIFI_SERVER_H_

#include "Print.h"
#include "Server.h"

class PinoccioWifiClient;

class PinoccioWifiServer : public Server {
  private:
    uint16_t _port;
    uint8_t protocol;
    void accept();
  public:
    PinoccioWifiServer(uint16_t, uint8_t);
    PinoccioWifiClient available();
    virtual void begin();
    virtual size_t write(uint8_t);
    virtual size_t write(const char *str);
    virtual size_t write(const uint8_t *buf, size_t size);
    using Print::write;
};

#endif // _PINOCCIO_WEB_WIFI_SERVER_H_