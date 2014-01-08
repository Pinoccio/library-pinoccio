#ifndef LIB_PINOCCIO_SHELL_H_
#define LIB_PINOCCIO_SHELL_H_

#include <Pinoccio.h>

class PinoccioShell {

  public:
    PinoccioShell();
    ~PinoccioShell();

    void setup();
    void loop();

    void startShell();
    void disableShell();

    byte pingCounter;
    NWK_DataReq_t pingDataReq;
    NWK_DataReq_t sendDataReq;
    bool sendDataReqBusy = false;
    bool isMeshVerbose = false;
  protected:

     bool shellEnabled;
};

#endif