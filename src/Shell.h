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

#include <electron.h>
#include "Scout.h"

#include <lwm.h>
#include "lwm/sys/sysConfig.h"
#include "lwm/phy/phy.h"
#include "lwm/hal/hal.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "lwm/sys/sysTimer.h"
#include "peripherals/halTemperature.h"
#include "avr/sleep.h"
#include "util/StringBuffer.h"
#include "util/Concatenator.h"

class PinoccioShell {

  public:
    PinoccioShell();
    ~PinoccioShell();

    void setup();
    void loop();
    void addFunction(const char *name, numvar (*func)(void));

    /**
     * The eval function takes care of evaluating a piece of
     * ScoutScript and is available in a few different versions.
     *
     * The most basic version just accepts a single String (or through
     * implicit conversion, const char*) containing a full command to
     * run:
     *
     *  Shell.eval("print 1");
     *  Shell.eval("function foo { print \"bar\"; }");
     *
     * Alternatively, you can pass a function name and arguments:
     *
     *  Shell.eval("wifi.config", "foo", "bar");
     *  Shell.eval("hq.verbose", 1);
     *
     * This constructs and evalutes wifi.config("foo", "bar") and
     * hq.verbose(1) respectively.
     *
     * String arguments are automatically quoted and passed to bitlash
     * as strings, integers are used as-is. To pass an integer as a
     * string argument, convert it to String explicitely:
     *
     *  Shell.eval("hq.report", "test", String(2));
     *
     * Shell.eval always returns the (numeric) result of the command.
     * The output is printed to the default location (usually Serial).
     * To redirect the output, pass a Print reference or pointer as the
     * first argument:
     *
     *  Shell.eval(Serial1, "uptime.status");
     *
     * To capture the output in a string, use the PrintToString class:
     *
     *  String output;
     *  Shell.eval(PrintToString(output), "pin.status");
     */

    /** Basic version that doesn't redirect output */
    template <typename... Args>
    numvar eval(const String& cmd, const Args&... args...) {
      return eval((Print*)NULL, cmd, args...);
    }

    /** Version accepting a Stream reference instead of a pointer */
    template <typename... Args>
    numvar eval(Print& out, const String &cmd, const Args&... args...) {
      return eval(&out, cmd, args...);
    }

    /** Version accepting a Stream rvalue reference instead of a pointer */
    template <typename... Args>
    numvar eval(Print&& out, const String &cmd, const Args&... args...) {
      return eval(&out, cmd, args...);
    }

    /** Version with arguments */
    template <typename... Args>
    numvar eval(Print *out, const String& cmd, const Args&... args...);

    /** Version without arguments */
    numvar eval(Print *out, const String& cmd);

    void allReportHQ();
    void delay(uint32_t at, const __FlashStringHelper *command);
    void delay(uint32_t at, char *command);
    bool defined(const char *fn);

    void startShell();
    void disableShell();
    void prompt();
    void refresh();
    void print(const char *str);
    bool outWait;
    bool isVerbose;
    bool isMuted;
    int lastMeshRssi;
    int lastMeshLqi;
    int lastMeshFrom;

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

// handle printing a float automagically
void sp(float f, int pre);
void speol(float f, int pre);

void bitlashFilter(byte b); // watches bitlash output for channel announcements
bool checkArgs(uint8_t min, uint8_t max, const __FlashStringHelper *errorMsg);
bool checkArgs(uint8_t exactly, const __FlashStringHelper *errorMsg);

template <typename... Args>
numvar PinoccioShell::eval(Print *out, const String &cmd, const Args&... args...) {
  StringBuffer buf(128);
  buf.concat(cmd);
  buf.concat('(');
  Concatenator<QuoteStringsAndFloats>::concat(buf, ',', args...);
  buf.concat(')');
  // This calls the no-argument version below, which just runs the
  // command
  return eval(out, buf);
}

static Print* evalOutput;
static void evalPrint(uint8_t c) {
  if(!evalOutput) return; // silent mode
  evalOutput->write(c);
}

static e_t ele_stack = NULL;
inline numvar PinoccioShell::eval(Print *out, const String &cmd) {
    Serial.print(F("running eval of: "));
    Serial.println((char*)cmd.c_str());

  ele_stack = e_val(ele_stack, (char*)cmd.c_str(), 0);
  out->write(e_vchar(ele_stack),e_vlen(ele_stack));

  return 0;
}

#endif
