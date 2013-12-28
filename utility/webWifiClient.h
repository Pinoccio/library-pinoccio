#ifndef _PINOCCIO_WEB_WIFI_CLIENT_H_
#define _PINOCCIO_WEB_WIFI_CLIENT_H_

#include <Arduino.h>
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"

class PinoccioWifiClient : public Client {
  public:
    PinoccioWifiClient();
    PinoccioWifiClient(uint8_t);

    uint8_t status();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(IPAddress ip, uint16_t port, uint8_t protocol);
    virtual int connect(const char *host, uint16_t port);
    virtual int autoConnect();

    virtual size_t write(uint8_t);
    virtual size_t write(const char *str);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool();
    void startDataTx();
    void stopDataTx();
    virtual bool enableTls(String certname);

    friend class webWifiServer;

    using Print::write;

  private:
    static uint16_t _srcport;
    uint8_t _sock;
    uint8_t _protocol;
};

#endif // _PINOCCIO_WEB_WIFI_CLIENT_H_