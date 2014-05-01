#ifndef LIB_PINOCCIO_SCOUTHANDLER_H_
#define LIB_PINOCCIO_SCOUTHANDLER_H_

#include <Pinoccio.h>
#include <ScoutHandler.h>
#include "util/StringBuffer.h"

class PinoccioScoutHandler {

  public:
    PinoccioScoutHandler();
    ~PinoccioScoutHandler();

    void setup();
    void loop();
    void announce(uint16_t group, const String& message);
    void setVerbose(bool flag);
    StringBuffer report(const String& report);
    bool isOnline();

  protected:
};

#endif
