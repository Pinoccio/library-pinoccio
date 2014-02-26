#ifndef LIB_PINOCCIO_SHELL_H_
#define LIB_PINOCCIO_SHELL_H_

#include "bitlash.h"
#include "src/bitlash.h"

#include "lwm/sys/sysConfig.h"
#include "lwm/phy/phy.h"
#include "lwm/hal/hal.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "lwm/sys/sysTimer.h"
#include "utility/halTemperature.h"
#include "avr/sleep.h"

class PinoccioShell {

  public:
    PinoccioShell();
    ~PinoccioShell();

    void setup();
    void loop();
    void allReportHQ();

    void startShell();
    void disableShell();

    char *bitlashOutput;
    /**
     * Parse a single hexadecimal character. Supports both uppercase and
     * lowercase A-Z. If the character is not a valid hex character,
     * calls bitlash fatal() and does not return, so this should only be
     * called from inside a bitlash command.
     */
    static uint8_t parseHex(char c);

    /**
     * Parse a hexadecimal string. Supports both uppercase and
     * lowercase A-Z.
     *
     *
     * The length passed is the (expected) length of the string. The
     * out buffer passed in should be at least length / 2 bytes long.
     *
     * If the length passed in is not even, the string passed in is
     * not exactly length characters long or any of the digits is not a
     * valid hex digit, this calls bitlash fatal() and does not return,
     * so this should only be called from inside a bitlash command.
     */
    static void parseHex(const char *str, size_t length, uint8_t *out);
  protected:
    bool isShellEnabled;
};

extern PinoccioShell Shell;

void bitlashFilter(byte b); // watches bitlash output for channel announcements
void bitlashBuffer(byte b); // buffers bitlash output from a command

#endif
