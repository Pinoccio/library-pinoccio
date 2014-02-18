#ifndef LIB_PINOCCIO_SCOUTHANDLER_H_
#define LIB_PINOCCIO_SCOUTHANDLER_H_

#include <Pinoccio.h>
#include <ScoutHandler.h>

class PinoccioScoutHandler {

  public:
    PinoccioScoutHandler();
    ~PinoccioScoutHandler();

    void setup();
    void loop();
    void announce(uint16_t group, char *message);
    void setVerbose(bool flag);
    char *report(char *report);

  protected:
};

void leadHQConnect();

#endif
