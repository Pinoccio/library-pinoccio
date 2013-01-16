#ifndef _PINOCCIO_WEB_WIFI_CLIENT_H_
#define _PINOCCIO_WEB_WIFI_CLIENT_H_

#include <Arduino.h>
#include "Print.h"

class PinoccioWifiClient : public Stream {
  public:
    PinoccioWifiClient();
    PinoccioWifiClient(uint8_t);
    PinoccioWifiClient(String, String, uint8_t);

    uint8_t status();
    uint8_t connect();

    //uint8_t connect();
    virtual size_t write(uint8_t);
    virtual size_t write(const char *str);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int peek();
    virtual void flush();
    operator bool();
    uint8_t connected();
    void stop();
    void startDataTx();
    void stopDataTx();
  #if 0
    uint8_t operator==(int);
    uint8_t operator!=(int);
  #endif

    friend class webWifiServer;

  private:
    static uint16_t _srcport;
    uint8_t _sock;
    String _port;
    String _ip;
    uint8_t _protocol;
};

#endif // _PINOCCIO_WEB_WIFI_CLIENT_H_