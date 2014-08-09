/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#ifndef LIB_PINOCCIO_H_
#define LIB_PINOCCIO_H_

//#define PINOCCIO_DEBUG
#ifdef PINOCCIO_DEBUG
#  define D(x) x
#else
#  define D(x)
#endif

#include <Arduino.h>

#include "lwm/sys/sysConfig.h"
#include "lwm/hal/hal.h"
#include "lwm/sys/sys.h"
#include "lwm/nwk/nwk.h"
#include "lwm/sys/sysTimer.h"
#include "peripherals/halTemperature.h"
#include "avr/sleep.h"

class PinoccioClass {

  public:
    PinoccioClass();
    ~PinoccioClass();

    void setup(const char *sketchName, const char *sketchRevision, int32_t sketchBuild);
    void loop();

    void goToSleep(uint32_t sleepForMs);

    int8_t getTemperature();
    int8_t getTemperatureOffset(void);
    void setTemperatureOffset(int8_t offset);
    void enableExternalAref();
    void disableExternalAref();
    bool getExternalAref();

    const char* getLastResetCause();
    void loadSettingsFromEeprom();

    void setHQToken(const char *token);
    void getHQToken(char *token);
    void resetHQToken();
    void setOTAFlag();

    uint32_t getHwSerial();
    uint16_t getHwFamily();
    uint8_t getHwVersion();
    uint8_t getEEPROMVersion();

    void sendStateToHQ();

    void meshSetRadio(const uint16_t theAddress, const uint16_t thePanId=0x4567, const uint8_t theChannel=20);
    void meshSetPower(const uint8_t theTxPower);
    void meshSetDataRate(const uint8_t theRate);
    void meshSetSecurityKey(const uint8_t *key);
    void meshGetSecurityKey(char *key);
    void meshResetSecurityKey(void);
    void meshListen(uint8_t endpoint, bool (*handler)(NWK_DataInd_t *ind));

    void meshJoinGroup(uint16_t groupAddress);
    void meshLeaveGroup(uint16_t groupAddress);
    bool meshIsInGroup(uint16_t groupAddress);

    uint16_t getAddress();
    uint16_t getPanId();
    uint8_t getChannel();
    uint8_t getTxPower();
    const char* getTxPowerDb();
    uint8_t getDataRate();
    const char* getDataRatekbps();

    const char* getSketchName();
    const char* getSketchRevision();
    int32_t getSketchBuild();

  protected:
    void convertLongToBytes(byte *convBytes, uint32_t target);
    uint32_t convertBytesToLong(byte *convBytes);

    bool isExternalAref;
    uint8_t lastResetCause;
    uint16_t address;
    uint16_t panId;
    uint8_t channel;
    uint8_t txPower;
    uint8_t dataRate;
    int8_t tempOffset;

    // Name of the sketch (e.g. "Bootstrap")
    const char* sketchName;
    // Detailed revision of the sketch (e.g. "2014031902-1-g5579a21-dirty")
    const char* sketchRevision;
    // Released build number, or -1 for custom builds
    int32_t sketchBuild;
};

#endif
