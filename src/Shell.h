/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
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
#include "peripherals/halTemperature.h"
#include "avr/sleep.h"
#include "util/StringBuffer.h"

class PinoccioShell {

  public:
    PinoccioShell();
    ~PinoccioShell();

    void setup();
    void loop();
    void addFunction(const char *name, numvar (*func)(void));
    numvar eval(const char *str, StringBuffer result);
    void allReportHQ();
    void delay(uint32_t at, char *command);
    bool defined(const char *fn);

    void startShell();
    void disableShell();
    void prompt();
    void refresh();
    void print(const char *str);
    bool outWait;

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
bool checkArgs(uint8_t min, uint8_t max, const __FlashStringHelper *errorMsg);
bool checkArgs(uint8_t exactly, const __FlashStringHelper *errorMsg);

#endif
