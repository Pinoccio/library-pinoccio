/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
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
    void setBridgeMode(bool flag);
    bool sendCommand(char *command, int to, int id);
    StringBuffer report(const String& report);

  protected:
};

void leadHQConnect();

#endif
