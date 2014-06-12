/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include "Shell.h"
#include "Scout.h"
#include "backpacks/Backpacks.h"
#include "bitlash.h"
#include "src/bitlash.h"
extern "C" {
#include "key/key.h"
#include "util/memdebug.h"
#include "util/StringBuffer.h"
}

static numvar pinoccioBanner(void);

static numvar getTemperatureC(void);
static numvar getTemperatureF(void);
static numvar temperatureReport(void);
static numvar getRandomNumber(void);

static numvar allReport(void);
static numvar allVerbose(void);

static numvar uptimeMillisAwake(void);
static numvar uptimeMillisSleep(void);
static numvar uptimeMillis(void);
static numvar uptimeSeconds(void);
static numvar uptimeMinutes(void);
static numvar uptimeHours(void);
static numvar uptimeDays(void);
static numvar uptimeReport(void);
static numvar getLastResetCause(void);

static numvar isBatteryCharging(void);
static numvar isBatteryConnected(void);
static numvar getBatteryPercentage(void);
static numvar getBatteryVoltage(void);
static numvar enableBackpackVcc(void);
static numvar disableBackpackVcc(void);
static numvar isBackpackVccEnabled(void);
static numvar sleep(void);
static numvar powerReport(void);

static numvar ledBlink(void);
static numvar ledOff(void);
static numvar ledRed(void);
static numvar ledGreen(void);
static numvar ledBlue(void);
static numvar ledCyan(void);
static numvar ledPurple(void);
static numvar ledMagenta(void);
static numvar ledYellow(void);
static numvar ledOrange(void);
static numvar ledWhite(void);
static numvar ledTorch(void);
static numvar ledSetHex(void);
static numvar ledGetHex(void);
static numvar ledSetRgb(void);
static numvar ledIsOff(void);
static numvar ledSaveTorch(void);
static numvar ledReport(void);

static numvar meshConfig(void);
static numvar meshSetPower(void);
static numvar meshSetDataRate(void);
static numvar meshSetKey(void);
static numvar meshGetKey(void);
static numvar meshResetKey(void);
static numvar meshJoinGroup(void);
static numvar meshLeaveGroup(void);
static numvar meshIsInGroup(void);
static numvar meshVerbose(void);
static numvar meshReport(void);
static numvar meshRouting(void);
static numvar meshSignal(void);
static numvar meshLoss(void);

static numvar messageScout(void);
static numvar messageGroup(void);

static numvar pinConstHigh(void);
static numvar pinConstLow(void);
static numvar pinConstDisabled(void);
static numvar pinConstInput(void);
static numvar pinConstOutput(void);
static numvar pinConstInputPullup(void);
static numvar pinMakeInput(void);
static numvar pinMakeOutput(void);
static numvar pinDisable(void);
static numvar pinSetMode(void);
static numvar pinRead(void);
static numvar pinWrite(void);
static numvar pinSave(void);
static numvar digitalPinReport(void);
static numvar analogPinReport(void);

static numvar backpackReport(void);
static numvar backpackList(void);
static numvar backpackEeprom(void);
static numvar backpackUpdateEeprom(void);
static numvar backpackDetail(void);
static numvar backpackResources(void);

static numvar scoutReport(void);
static numvar isScoutLeadScout(void);
static numvar setHQToken(void);
static numvar getHQToken(void);
static numvar scoutDelay(void);
static numvar memoryReport(void);
static numvar daisyWipe(void);
static numvar boot(void);
static numvar otaBoot(void);

static numvar hqVerbose(void);
static numvar hqPrint(void);
static numvar hqReport(void);
static numvar hqBridge(void);
static numvar hqBridgeCommand(void);

static numvar startStateChangeEvents(void);
static numvar stopStateChangeEvents(void);
static numvar setEventCycle(void);
static numvar setEventVerbose(void);

static numvar wifiReport(void);
static numvar wifiStatus(void);
static numvar wifiList(void);
static numvar wifiConfig(void);
static numvar wifiDhcp(void);
static numvar wifiStatic(void);
static numvar wifiReassociate(void);
static numvar wifiDisassociate(void);
static numvar wifiCommand(void);
static numvar wifiPing(void);
static numvar wifiDNSLookup(void);
static numvar wifiGetTime(void);
static numvar wifiSleep(void);
static numvar wifiWakeup(void);
static numvar wifiVerbose(void);
static numvar wifiStats(void);

static numvar keyMap(void);
static numvar keyFree(void);
static numvar keyPrint(void);
static numvar keyNumber(void);
static numvar keySave(void);

static int getPinFromArg(int arg);
static bool checkArgs(uint8_t min, uint8_t max, const __FlashStringHelper *errorMsg);
static bool checkArgs(uint8_t exactly, const __FlashStringHelper *errorMsg);

static StringBuffer scoutReportHQ(void);
static StringBuffer uptimeReportHQ(void);
static StringBuffer powerReportHQ(void);
static StringBuffer backpackReportHQ(void);
static StringBuffer digitalPinReportHQ(void);
static StringBuffer analogPinReportHQ(void);
static StringBuffer meshReportHQ(void);
static StringBuffer tempReportHQ(void);
static StringBuffer ledReportHQ(void);

static bool receiveMessage(NWK_DataInd_t *ind);
static void sendMessage(int address, const String&);
static void sendConfirm(NWK_DataReq_t *req);

static void digitalPinEventHandler(uint8_t pin, int8_t value, int8_t mode);
static void analogPinEventHandler(uint8_t pin, int16_t value, int8_t mode);
static void batteryPercentageEventHandler(uint8_t value);
static void batteryChargingEventHandler(uint8_t value);
static void temperatureEventHandler(int8_t tempC, int8_t tempF);
static void ledEventHandler(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);

PinoccioShell Shell;

PinoccioShell::PinoccioShell() {
  isShellEnabled = true;
}

PinoccioShell::~PinoccioShell() { }

void PinoccioShell::setup() {
  keyInit();
  // This overrides the normal banner
  addBitlashFunction("banner", (bitlash_function) pinoccioBanner);

  addBitlashFunction("power.ischarging", (bitlash_function) isBatteryCharging);
  addBitlashFunction("power.hasbattery", (bitlash_function) isBatteryConnected);
  addBitlashFunction("power.percent", (bitlash_function) getBatteryPercentage);
  addBitlashFunction("power.voltage", (bitlash_function) getBatteryVoltage);
  addBitlashFunction("power.enablevcc", (bitlash_function) enableBackpackVcc);
  addBitlashFunction("power.disablevcc", (bitlash_function) disableBackpackVcc);
  addBitlashFunction("power.isvccenabled", (bitlash_function) isBackpackVccEnabled);
  addBitlashFunction("power.sleep", (bitlash_function) sleep);
  addBitlashFunction("power.report", (bitlash_function) powerReport);

  addBitlashFunction("mesh.config", (bitlash_function) meshConfig);
  addBitlashFunction("mesh.setpower", (bitlash_function) meshSetPower);
  addBitlashFunction("mesh.setdatarate", (bitlash_function) meshSetDataRate);
  addBitlashFunction("mesh.setkey", (bitlash_function) meshSetKey);
  addBitlashFunction("mesh.getkey", (bitlash_function) meshGetKey);
  addBitlashFunction("mesh.resetkey", (bitlash_function) meshResetKey);
  addBitlashFunction("mesh.joingroup", (bitlash_function) meshJoinGroup);
  addBitlashFunction("mesh.leavegroup", (bitlash_function) meshLeaveGroup);
  addBitlashFunction("mesh.ingroup", (bitlash_function) meshIsInGroup);
  addBitlashFunction("mesh.verbose", (bitlash_function) meshVerbose);
  addBitlashFunction("mesh.report", (bitlash_function) meshReport);
  addBitlashFunction("mesh.routing", (bitlash_function) meshRouting);
  addBitlashFunction("mesh.signal", (bitlash_function) meshSignal);
  addBitlashFunction("mesh.loss", (bitlash_function) meshLoss);

  addBitlashFunction("message.scout", (bitlash_function) messageScout);
  addBitlashFunction("message.group", (bitlash_function) messageGroup);

  addBitlashFunction("temperature.c", (bitlash_function) getTemperatureC);
  addBitlashFunction("temperature.f", (bitlash_function) getTemperatureF);
  addBitlashFunction("temperature.report", (bitlash_function) temperatureReport);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);
  addBitlashFunction("memory.report", (bitlash_function) memoryReport);

  addBitlashFunction("report", (bitlash_function) allReport);
  addBitlashFunction("verbose", (bitlash_function) allVerbose);

  addBitlashFunction("uptime.millis.awake", (bitlash_function) uptimeMillisAwake);
  addBitlashFunction("uptime.millis.sleep", (bitlash_function) uptimeMillisSleep);
  addBitlashFunction("uptime.millis", (bitlash_function) uptimeMillis);
  addBitlashFunction("uptime.seconds", (bitlash_function) uptimeSeconds);
  addBitlashFunction("uptime.minutes", (bitlash_function) uptimeMinutes);
  addBitlashFunction("uptime.hours", (bitlash_function) uptimeHours);
  addBitlashFunction("uptime.days", (bitlash_function) uptimeDays);
  addBitlashFunction("uptime.report", (bitlash_function) uptimeReport);
  addBitlashFunction("uptime.getlastreset", (bitlash_function) getLastResetCause);

  addBitlashFunction("led.on", (bitlash_function) ledTorch); // alias
  addBitlashFunction("led.off", (bitlash_function) ledOff);
  addBitlashFunction("led.red", (bitlash_function) ledRed);
  addBitlashFunction("led.green", (bitlash_function) ledGreen);
  addBitlashFunction("led.blue", (bitlash_function) ledBlue);
  addBitlashFunction("led.cyan", (bitlash_function) ledCyan);
  addBitlashFunction("led.purple", (bitlash_function) ledPurple);
  addBitlashFunction("led.magenta", (bitlash_function) ledMagenta);
  addBitlashFunction("led.yellow", (bitlash_function) ledYellow);
  addBitlashFunction("led.orange", (bitlash_function) ledOrange);
  addBitlashFunction("led.white", (bitlash_function) ledWhite);
  addBitlashFunction("led.torch", (bitlash_function) ledTorch);
  addBitlashFunction("led.blink", (bitlash_function) ledBlink);
  addBitlashFunction("led.sethex", (bitlash_function) ledSetHex);
  addBitlashFunction("led.gethex", (bitlash_function) ledGetHex);
  addBitlashFunction("led.setrgb", (bitlash_function) ledSetRgb);
  addBitlashFunction("led.isoff", (bitlash_function) ledIsOff);
  addBitlashFunction("led.savetorch", (bitlash_function) ledSaveTorch);
  addBitlashFunction("led.report", (bitlash_function) ledReport);

  addBitlashFunction("high", (bitlash_function) pinConstHigh);
  addBitlashFunction("low", (bitlash_function) pinConstLow);
  addBitlashFunction("disabled", (bitlash_function) pinConstDisabled);
  addBitlashFunction("input", (bitlash_function) pinConstInput);
  addBitlashFunction("output", (bitlash_function) pinConstOutput);
  addBitlashFunction("input_pullup", (bitlash_function) pinConstInputPullup);

  addBitlashFunction("pin.makeinput", (bitlash_function) pinMakeInput);
  addBitlashFunction("pin.makeoutput", (bitlash_function) pinMakeOutput);
  addBitlashFunction("pin.disable", (bitlash_function) pinDisable);
  addBitlashFunction("pin.setmode", (bitlash_function) pinSetMode);
  addBitlashFunction("pin.read", (bitlash_function) pinRead);
  addBitlashFunction("pin.write", (bitlash_function) pinWrite);
  addBitlashFunction("pin.save", (bitlash_function) pinSave);
  addBitlashFunction("pin.report.digital", (bitlash_function) digitalPinReport);
  addBitlashFunction("pin.report.analog", (bitlash_function) analogPinReport);

  addBitlashFunction("backpack.report", (bitlash_function) backpackReport);
  addBitlashFunction("backpack.list", (bitlash_function) backpackList);
  addBitlashFunction("backpack.eeprom", (bitlash_function) backpackEeprom);
  addBitlashFunction("backpack.eeprom.update", (bitlash_function) backpackUpdateEeprom);
  addBitlashFunction("backpack.detail", (bitlash_function) backpackDetail);
  addBitlashFunction("backpack.resources", (bitlash_function) backpackResources);

  addBitlashFunction("scout.report", (bitlash_function) scoutReport);
  addBitlashFunction("scout.isleadscout", (bitlash_function) isScoutLeadScout);
  addBitlashFunction("scout.delay", (bitlash_function) scoutDelay);
  addBitlashFunction("scout.daisy", (bitlash_function) daisyWipe);
  addBitlashFunction("scout.boot", (bitlash_function) boot);
  addBitlashFunction("scout.otaboot", (bitlash_function) otaBoot);

  addBitlashFunction("hq.settoken", (bitlash_function) setHQToken);
  addBitlashFunction("hq.gettoken", (bitlash_function) getHQToken);
  addBitlashFunction("hq.verbose", (bitlash_function) hqVerbose);
  addBitlashFunction("hq.print", (bitlash_function) hqPrint);
  addBitlashFunction("hq.report", (bitlash_function) hqReport);
  addBitlashFunction("hq.bridge", (bitlash_function) hqBridge);
  addBitlashFunction("hq.bridge.command", (bitlash_function) hqBridgeCommand);

  addBitlashFunction("events.start", (bitlash_function) startStateChangeEvents);
  addBitlashFunction("events.stop", (bitlash_function) stopStateChangeEvents);
  addBitlashFunction("events.setcycle", (bitlash_function) setEventCycle);
  addBitlashFunction("events.verbose", (bitlash_function) setEventVerbose);

  addBitlashFunction("key", (bitlash_function) keyMap);
  addBitlashFunction("key.free", (bitlash_function) keyFree);
  addBitlashFunction("key.print", (bitlash_function) keyPrint);
  addBitlashFunction("key.number", (bitlash_function) keyNumber);
  addBitlashFunction("key.save", (bitlash_function) keySave);

  if (Scout.isLeadScout()) {
    addBitlashFunction("wifi.report", (bitlash_function) wifiReport);
    addBitlashFunction("wifi.status", (bitlash_function) wifiStatus);
    addBitlashFunction("wifi.list", (bitlash_function) wifiList);
    addBitlashFunction("wifi.config", (bitlash_function) wifiConfig);
    addBitlashFunction("wifi.dhcp", (bitlash_function) wifiDhcp);
    addBitlashFunction("wifi.static", (bitlash_function) wifiStatic);
    addBitlashFunction("wifi.reassociate", (bitlash_function) wifiReassociate);
    addBitlashFunction("wifi.disassociate", (bitlash_function) wifiDisassociate);
    addBitlashFunction("wifi.command", (bitlash_function) wifiCommand);
    addBitlashFunction("wifi.ping", (bitlash_function) wifiPing);
    addBitlashFunction("wifi.dnslookup", (bitlash_function) wifiDNSLookup);
    addBitlashFunction("wifi.gettime", (bitlash_function) wifiGetTime);
    addBitlashFunction("wifi.sleep", (bitlash_function) wifiSleep);
    addBitlashFunction("wifi.wakeup", (bitlash_function) wifiWakeup);
    addBitlashFunction("wifi.verbose", (bitlash_function) wifiVerbose);
    addBitlashFunction("wifi.stats", (bitlash_function) wifiStats);
  }

  // set up event handlers
  Scout.digitalPinEventHandler = digitalPinEventHandler;
  Scout.analogPinEventHandler = analogPinEventHandler;
  Scout.batteryPercentageEventHandler = batteryPercentageEventHandler;
  Scout.batteryChargingEventHandler = batteryChargingEventHandler;
  Scout.temperatureEventHandler = temperatureEventHandler;
  Led.ledEventHandler = ledEventHandler;

  if (isShellEnabled) {
    startShell();
  } else {
    Serial.begin(115200);
  }

  Scout.meshListen(1, receiveMessage);

  if (!Scout.isLeadScout()) {
    Shell.allReportHQ(); // lead scout reports on hq connect
  }
}

static bool isMeshVerbose;
static bool isBridgeMode;
static int lastMeshRssi = 0;
static int lastMeshLqi = 0;

// report all transient settings when asked
void PinoccioShell::allReportHQ() {
  scoutReportHQ();
  uptimeReportHQ();
  powerReportHQ();
  backpackReportHQ();
  digitalPinReportHQ();
  analogPinReportHQ();
  meshReportHQ();
  tempReportHQ();
  ledReportHQ();
}

uint8_t PinoccioShell::parseHex(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }

  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 10;
  }

  if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 10;
  }
  // TODO: Better error message
  unexpected(M_number);
}

void PinoccioShell::parseHex(const char *str, size_t length, uint8_t *out) {
  // TODO: Better error message
  if (length % 2) {
    unexpected(M_number);
  }

  // Convert each digit in turn. If the string ends before we reach
  // length, parseHex will error out on the trailling \0
  for (size_t i = 0; i < length; i += 2) {
    out[i / 2] = parseHex(str[i]) << 4 | parseHex(str[i + 1]);
  }

  // See if the string is really finished
  // TODO: Better error message
  if (str[length]) {
    unexpected(M_number);
  }
}


static numvar pinoccioBanner(void) {
  speol(F("Hello from Pinoccio!"));
  speol(F(" (Shell based on Bitlash v2.0 (c) 2014 Bill Roy)"));
  sp(F(" "));
  sp(Scout.getSketchName());
  sp(F(" Sketch (rev "));
  sp(Scout.getSketchRevision());
  speol(F(")"));
  sp(F(" "));
  sp(func_free());
  speol(F(" bytes free"));

  if (Scout.isLeadScout()) {
    speol(F(" Lead Scout ready"));
  } else {
    speol(F(" Field Scout ready"));
  }
  return 1;
}

static numvar allReport(void) {
  speol(F("running all reports"));
  Shell.allReportHQ();
  return 1;
}

static numvar allVerbose(void) {
  Scout.handler.setVerbose(getarg(1));
  isMeshVerbose = getarg(1);
  Scout.eventVerboseOutput = getarg(1);
  return 1;
}

void PinoccioShell::loop() {
  if (isShellEnabled) {
    runBitlash();
    keyLoop(millis());
  }
}

void PinoccioShell::startShell() {
  char buf[32];
  uint8_t i;

  isShellEnabled = true;
  initBitlash(115200);

  for (i='a'; i<'z'; i++) {
    snprintf(buf, sizeof(buf), "startup.%c", i);
    if (find_user_function(buf) || findscript(buf)) {
      doCommand(buf);
    }
  }

  for (i=2; i<9; i++) {
    snprintf(buf, sizeof(buf), "startup.d%d", i);
    if (find_user_function(buf) || findscript(buf)) {
      doCommand(buf);
    }
  }

  for (i=0; i<8; i++) {
    snprintf(buf, sizeof(buf), "startup.a%d", i);
    if (find_user_function(buf) || findscript(buf)) {
      doCommand(buf);
    }
  }
}

void PinoccioShell::disableShell() {
  isShellEnabled = false;
}

static NWK_DataReq_t sendDataReq;
static bool sendDataReqBusy;

/****************************\
*      BUILT-IN HANDLERS    *
\****************************/

static StringBuffer tempReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[%d,%d]]",
          keyMap("temp", 0),
          keyMap("c", 0),
          keyMap("f", 0),
          Scout.getTemperatureC(),
          Scout.getTemperatureF());
  return Scout.handler.report(report);
}

static numvar temperatureReport(void) {
  speol(tempReportHQ());
  return 1;
}

static numvar getTemperatureC(void) {
  return Scout.getTemperatureC();
}

static numvar getTemperatureF(void) {
  return Scout.getTemperatureF();
}

static numvar getRandomNumber(void) {
  int i = random();
  return i;
}

static numvar getLastResetCause(void) {
  char c;
  char reset[20];
  const char *resetString = Scout.getLastResetCause();
  reset[0] = 0;

  while((c = pgm_read_byte(resetString++))) {
    sprintf(reset + strlen(reset), "%c", c);
  }
  return keyMap(reset, 0);
}

static StringBuffer uptimeReportHQ(void) {
  StringBuffer report(100);
  int freeMem = getFreeMemory();

  char reset[20];
  strncpy_P(reset, Scout.getLastResetCause(), sizeof(reset));
  reset[sizeof(reset) - 1] = 0; // ensure termination, strncpy is weird

  report.appendSprintf("[%d,[%d,%d,%d,%d],[%ld,%ld,%d,",keyMap("uptime",0),
          keyMap("millis", 0),
          keyMap("sleep", 0),
          keyMap("random", 0),
          keyMap("reset", 0),
          Scout.getCpuTime(),
          Scout.getSleepTime(),
          (int)random());

  report.appendJsonString(reset, true);
  report += "]]";
  return Scout.handler.report(report);
}

static numvar uptimeMillis(void) {
  return Scout.getWallTime();
}

static numvar uptimeSeconds(void) {
  return Scout.getWallTime()/1000;
}

static numvar uptimeMinutes(void) {
  return Scout.getWallTime()/1000/60;
}

static numvar uptimeHours(void) {
  return Scout.getWallTime()/1000/60/60;
}

static numvar uptimeDays(void) {
  return Scout.getWallTime()/1000/60/60/24;
}

static numvar uptimeReport(void) {
  speol(uptimeReportHQ());
  return true;
}

static numvar uptimeMillisAwake(void) {
  return Scout.getCpuTime();
  return true;
}

static numvar uptimeMillisSleep(void) {
  return Scout.getSleepTime();
  return true;
}

/****************************\
*        KEY HANDLERS        *
\****************************/

static numvar keyMap(void) {
  if (!checkArgs(1, F("usage: key(\"string\")"))) {
    return 0;
  }
  static char num[8];
  if (isstringarg(1)) {
    return keyMap((char*)getstringarg(1), 0);
  }
  snprintf(num, 8, "%lu", getarg(1));
  return keyMap(num, 0);
}

static numvar keyFree(void) {
  if (!checkArgs(1, F("usage: key.free(key)"))) {
    return 0;
  }
  keyFree(getarg(1));
  return 1;
}

static numvar keyPrint(void) {
  if (!checkArgs(1, F("usage: key.print(key)"))) {
    return 0;
  }
  const char *key = keyGet(getarg(1));
  if (!key) {
    return 0;
  }
  speol(key);
  return 1;
}

static numvar keyNumber(void) {
  if (!checkArgs(1, F("usage: key.number(key)"))) {
    return 0;
  }
  const char *key = keyGet(getarg(1));
  if (!key) {
    return 0;
  }
  return atoi(key);
}

static numvar keySave(void) {
  if (!checkArgs(2, F("usage: key.save(\"string\", at)")) || !isstringarg(1)) {
    return 0;
  }
  char cmd[42], *var;
  var = (char*)getstringarg(1);
  snprintf(cmd, sizeof(cmd), "function boot.%s {%s=key(\"%s\");}", var, var, keyGet(getarg(2)));
  doCommand(cmd);
  return 1;
}


/****************************\
*      POWER HANDLERS       *
\****************************/

static numvar isBatteryCharging(void) {
  return Scout.isBatteryCharging();
}

static numvar isBatteryConnected(void) {
  return Scout.isBatteryConnected();
}

static numvar getBatteryPercentage(void) {
  return Scout.getBatteryPercentage();
}

static numvar getBatteryVoltage(void) {
  return Scout.getBatteryVoltage();
}

static numvar enableBackpackVcc(void) {
  Scout.enableBackpackVcc();
  return 1;
}

static numvar disableBackpackVcc(void) {
  Scout.disableBackpackVcc();
  return 1;
}

static numvar isBackpackVccEnabled(void) {
  return Scout.isBackpackVccEnabled();
}

static numvar sleep(void) {
  if (!getarg(0) || getarg(0) > 2) {
    speol("usage: power.sleep(ms, [\"function\"])");
    return 0;
  }

  const char *cmd = NULL;
  if (getarg(0) > 1) {
    if (isstringarg(2))
      cmd = (char*)getstringarg(2);
    else
      cmd = keyGet(getarg(2));
  }
  Scout.scheduleSleep(getarg(1), strdup(cmd));

  return 1;
}

static StringBuffer powerReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d,%d,%d],[%d,%d,%s,%s]]",
          keyMap("power", 0),
          keyMap("battery", 0),
          keyMap("voltage", 0),
          keyMap("charging", 0),
          keyMap("vcc", 0),
          (int)Scout.getBatteryPercentage(),
          (int)Scout.getBatteryVoltage(),
          Scout.isBatteryCharging()?"true":"false",
          Scout.isBackpackVccEnabled()?"true":"false");
  return Scout.handler.report(report);
}

static numvar powerReport(void) {
  speol(powerReportHQ());
  return 1;
}

/****************************\
*      RGB LED HANDLERS     *
\****************************/

static StringBuffer ledReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[[%d,%d,%d],[%d,%d,%d]]]",
          keyMap("led", 0),
          keyMap("led", 0),
          keyMap("torch", 0),
          Led.getRedValue(),
          Led.getGreenValue(),
          Led.getBlueValue(),
          Led.getRedTorchValue(),
          Led.getGreenTorchValue(),
          Led.getBlueTorchValue());
  return Scout.handler.report(report);
}

static numvar ledBlink(void) {
  if (!checkArgs(3, 5, F("usage: led.blink(red, green, blue, ms=500, continuous=0)"))) {
    return 0;
  }
  if (getarg(0) == 5) {
    Led.blinkColor(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5));
  } else if (getarg(0) == 4) {
    Led.blinkColor(getarg(1), getarg(2), getarg(3), getarg(4));
  } else {
    Led.blinkColor(getarg(1), getarg(2), getarg(3));
  }
  return 1;
}

static numvar ledOff(void) {
  Led.turnOff();
  return 1;
}

static numvar ledRed(void) {
  if (getarg(0) == 2) {
    Led.blinkRed(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkRed(getarg(1));
  } else {
    Led.red();
  }
  return 1;
}

static numvar ledGreen(void) {
  if (getarg(0) == 2) {
    Led.blinkGreen(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkGreen(getarg(1));
  } else {
    Led.green();
  }
  return 1;
}

static numvar ledBlue(void) {
  if (getarg(0) == 2) {
    Led.blinkBlue(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkBlue(getarg(1));
  } else {
    Led.blue();
  }
  return 1;
}

static numvar ledCyan(void) {
  if (getarg(0) == 2) {
    Led.blinkCyan(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkCyan(getarg(1));
  } else {
    Led.cyan();
  }
  return 1;
}

static numvar ledPurple(void) {
  if (getarg(0) == 2) {
    Led.blinkPurple(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkPurple(getarg(1));
  } else {
    Led.purple();
  }
  return 1;
}

static numvar ledMagenta(void) {
  if (getarg(0) == 2) {
    Led.blinkMagenta(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkMagenta(getarg(1));
  } else {
    Led.magenta();
  }
  return 1;
}

static numvar ledYellow(void) {
  if (getarg(0) == 2) {
    Led.blinkYellow(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkYellow(getarg(1));
  } else {
    Led.yellow();
  }
  return 1;
}

static numvar ledOrange(void) {
  if (getarg(0) == 2) {
    Led.blinkOrange(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkOrange(getarg(1));
  } else {
    Led.orange();
  }
  return 1;
}

static numvar ledWhite(void) {
  if (getarg(0) == 2) {
    Led.blinkWhite(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkWhite(getarg(1));
  } else {
    Led.white();
  }
  return 1;
}

static numvar ledGetHex(void) {
  char hex[8];
  snprintf(hex, sizeof(hex),"%02x%02x%02x", Led.getRedValue(), Led.getGreenValue(), Led.getBlueValue());
  return keyMap(hex, millis());
}

static numvar ledSetHex(void) {
  if (!checkArgs(1, F("usage: led.sethex(\"hexvalue\")"))) {
    return 0;
  }

  const char *str;
  if (isstringarg(1)) {
    str = (const char *)getarg(1);
  } else {
    str = keyGet(getarg(1));
  }

  uint8_t out[3];
  PinoccioShell::parseHex(str, 6, out);
  Led.setColor(out[0], out[1], out[2]);

  return 1;
}

static numvar ledSetRgb(void) {
  if (!checkArgs(3, F("usage: led.setrgb(red, green, blue)"))) {
    return 0;
  }
  Led.setColor(getarg(1), getarg(2), getarg(3));
  return 1;
}

static numvar ledIsOff(void) {
  return Led.isOff();
}

static numvar ledSaveTorch(void) {
  if (!checkArgs(3, F("usage: led.savetorch(red, green, blue)"))) {
    return 0;
  }
  Led.saveTorch(getarg(1), getarg(2), getarg(3));
  return 1;
}

static numvar ledTorch(void) {
  if (getarg(0) == 2) {
    Led.blinkTorch(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkTorch(getarg(1));
  } else {
    Led.setTorch();
  }
  return 1;
}

static numvar ledReport(void) {
  speol(ledReportHQ());
  return 1;
}

/****************************\
*    MESH RADIO HANDLERS    *
\****************************/

static numvar meshConfig(void) {
  if (!checkArgs(2, 3, F("usage: mesh.config(scoutId, troopId, channel=20)"))) {
    return 0;
  }
  uint8_t channel = 20;

  if (getarg(0) >= 3) {
    channel = getarg(3);
  }
  Scout.meshSetRadio(getarg(1), getarg(2), channel);
  return 1;
}

static numvar meshSetPower(void) {
  if (!checkArgs(1, F("usage: mesh.setpower(powerLevel)"))) {
    return 0;
  }
  Scout.meshSetPower(getarg(1));
  return 1;
}

static numvar meshSetDataRate(void) {
  if (!checkArgs(1, F("usage: mesh.setdatarate(dataRate)"))) {
    return 0;
  }
  Scout.meshSetDataRate(getarg(1));
  return 1;
}

static numvar meshSetKey(void) {
  if (!checkArgs(1, F("usage: mesh.setkey(\"key\")"))) {
    return 0;
  }
  int len = strlen((const char*)getstringarg(1));
  char key[NWK_SECURITY_KEY_SIZE];
  memset(key, 0xFF, NWK_SECURITY_KEY_SIZE);
  if (len > NWK_SECURITY_KEY_SIZE) {
    len = NWK_SECURITY_KEY_SIZE;
  }
  memcpy(key, (const char*)getstringarg(1), len);
  Scout.meshSetSecurityKey((const uint8_t *)key);
  return 1;
}

static numvar meshGetKey(void) {
  char token[17];
  Scout.meshGetSecurityKey((char *)token);
  token[16] = 0;
  speol(token);
  return keyMap(token, millis());
}

static numvar meshResetKey(void) {
  Scout.meshResetSecurityKey();
  return 1;
}

static numvar meshJoinGroup(void) {
  if (!checkArgs(1, F("usage: mesh.joingroup(groupId)"))) {
    return 0;
  }
  Scout.meshJoinGroup(getarg(1));
  return 1;
}

static numvar meshLeaveGroup(void) {
  if (!checkArgs(1, F("usage: mesh.leavegroup(groupId)"))) {
    return 0;
  }
  Scout.meshLeaveGroup(getarg(1));
  return 1;
}

static numvar meshIsInGroup(void) {
  if (!checkArgs(1, F("usage: mesh.ingroup(groupId)"))) {
    return 0;
  }
  return Scout.meshIsInGroup(getarg(1));
}

// ver = 0 means all args, ver < 0 means skip first arg, ver >= 1 means include ver and skip first arg
StringBuffer arg2array(int ver) {
  StringBuffer buf(100);
  int i;
  int args = getarg(0);
  if (args > 8) {
    args = 8;
  }
  buf = "[";
  if(ver >= 0) buf.appendSprintf("%d,", ver);
  for (i=(ver!=0)?2:1; i<=args; i++) {
    int key = (isstringarg(i)) ? keyMap((char*)getstringarg(i), 0) : getarg(i);
    buf.appendJsonString(keyGet(key), true);
    if(i+1 <= args) buf += ",";
  }
  buf += "]";
  return buf;
}

static numvar meshSignal(void) {
  return lastMeshRssi * -1;
}

static numvar meshLoss(void) {
  return lastMeshLqi;
}

static numvar meshVerbose(void) {
  if (!checkArgs(1, F("usage: mesh.verbose(flag)"))) {
    return 0;
  }
  isMeshVerbose = getarg(1);
  return 1;
}

static StringBuffer meshReportHQ(void) {
  StringBuffer report(100);
  int count = 0;
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i<NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) continue;
    count++;
  }
  report.appendSprintf("[%d,[%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,\"",
          keyMap("mesh", 0),
          keyMap("scoutid", 0),
          keyMap("troopid", 0),
          keyMap("routes", 0),
          keyMap("channel", 0),
          keyMap("rate", 0),
          keyMap("power", 0),
          Scout.getAddress(),
          Scout.getPanId(),
          count,
          Scout.getChannel());

  const char *kbString = Scout.getDataRatekbps();
  while (char c = pgm_read_byte(kbString++)) {
    report += c;
  }

  report += "\",\"";

  const char *dbString = Scout.getTxPowerDb();
  while (char c = pgm_read_byte(dbString++)) {
    report += c;
  }
  report += "\"]]";

  return Scout.handler.report(report);
}

static numvar meshReport(void) {
  speol(meshReportHQ());
  return 1;
}

static numvar meshRouting(void) {
  sp(F("|    Fixed    |  Multicast  |    Score    |    DstAdd   | NextHopAddr |    Rank     |     LQI     |"));
  speol();
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i < NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) {
      continue;
    }
    sp(F("|      "));
    sp(table[i].fixed);
    sp(F("      |      "));
    sp(table[i].multicast);
    sp(F("      |      "));
    sp(table[i].score);
    sp(F("      |      "));
    sp(table[i].dstAddr);
    sp(F("      |      "));
    sp(table[i].nextHopAddr);
    sp(F("      |     "));
    sp(table[i].rank);
    sp(F("     |     "));
    sp(table[i].lqi);
    sp(F("     |"));
    speol();
  }
  return 1;
}

static numvar messageScout(void) {
  if (!checkArgs(1, 99, F("usage: message.scout(scoutId, \"message\")"))) {
    return 0;
  }
  sendMessage(getarg(1), arg2array(1));
  return 1;
}

static numvar messageGroup(void) {
  if (!checkArgs(1, 99, F("usage: message.group(groupId, \"message\")"))) {
    return 0;
  }
  Scout.handler.announce(getarg(1), arg2array(1));
  return 1;
}

/****************************\
*        I/O HANDLERS       *
\****************************/

static StringBuffer digitalPinReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d]]]",
          keyMap("digital", 0),
          keyMap("mode", 0),
          keyMap("state", 0),
          Scout.getPinMode(2),
          Scout.getPinMode(3),
          Scout.getPinMode(4),
          Scout.getPinMode(5),
          Scout.getPinMode(6),
          Scout.getPinMode(7),
          Scout.getPinMode(8),
          Scout.digitalPinState[0],
          Scout.digitalPinState[1],
          Scout.digitalPinState[2],
          Scout.digitalPinState[3],
          Scout.digitalPinState[4],
          Scout.digitalPinState[5],
          Scout.digitalPinState[6]);
  return Scout.handler.report(report);
}

static StringBuffer analogPinReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d,%d]]]",
          keyMap("analog", 0),
          keyMap("mode", 0),
          keyMap("state", 0),
          Scout.getPinMode(24),
          Scout.getPinMode(25),
          Scout.getPinMode(26),
          Scout.getPinMode(27),
          Scout.getPinMode(28),
          Scout.getPinMode(29),
          Scout.getPinMode(30),
          Scout.getPinMode(31),
          Scout.analogPinState[0],
          Scout.analogPinState[1],
          Scout.analogPinState[2],
          Scout.analogPinState[3],
          Scout.analogPinState[4],
          Scout.analogPinState[5],
          Scout.analogPinState[6],
          Scout.analogPinState[7]);
  return Scout.handler.report(report);
}

static numvar pinConstHigh(void) {
  return 1;
}

static numvar pinConstLow(void) {
  return 0;
}

static numvar pinConstDisabled(void) {
  return -1;
}

static numvar pinConstInput(void) {
  return 0;
}

static numvar pinConstOutput(void) {
  return 1;
}

static numvar pinConstInputPullup(void) {
  return 2;
}

static numvar pinMakeInput(void) {
  if (!checkArgs(1, 2, F("usage: pin.makeinput(\"pinName\", inputType=INPUT_PULLUP)"))) {
    return 0;
  }
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  bool pullup = true;
  if (getarg(0) == 2 && getarg(2) == 0) {
    pullup = false;
  }

  if (!Scout.makeInput(pin, pullup)) {
    speol(F("Cannot change mode of reserved pin"));
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
  return 1;
}

static numvar pinMakeOutput(void) {
  if (!checkArgs(1, F("usage: pin.makeoutput(\"pinName\")"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (!Scout.makeOutput(pin)) {
    speol(F("Cannot change mode of reserved pin"));
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
  return 1;
}

static numvar pinDisable(void) {
  if (!checkArgs(1, F("usage: pin.disable(\"pinName\")"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (!Scout.makeDisabled(pin)) {
    speol(F("Cannot change mode of reserved pin"));
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
  return 1;
}

static numvar pinSetMode(void) {
  if (!checkArgs(2, F("usage: pin.setmode(\"pinName\", pinMode)"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (!Scout.setMode(pin, getarg(2))) {
    speol(F("Cannot change mode of reserved pin"));
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
  return 1;
}

static numvar pinRead(void) {
  if (!checkArgs(1, F("usage: pin.read(\"pinName\")"))) {
    return 0;
  }
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  return Scout.pinRead(pin);
}

static numvar pinWrite(void) {
  // TODO: handle PWM pins
  if (!checkArgs(2, F("usage: pin.write(\"pinName\", pinValue)"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  uint8_t value = getarg(2);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (value < 0 || value > 1) {
    speol(F("Invalid pin value"));
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    Scout.pinWrite(pin, value);
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    Scout.pinWrite(pin, value);
    analogPinReportHQ();
  }
  return true;
}

static numvar pinSave(void) {
  if (!checkArgs(2, 3, F("usage: pin.save(\"pinName\", pinMode, [pinValue])"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  int8_t mode = getarg(2);

  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (Scout.isPinReserved(pin)) {
    speol(F("Cannot change mode of reserved pin"));
    return 0;
  }

  if (mode < -1 || mode > 2) {
    speol(F("Invalid pin mode"));
    return 0;
  }

  Scout.setMode(pin, mode);

  char buf[128];
  const char *str = (const char*)getstringarg(1);

  if (mode == -1) {
    snprintf(buf, sizeof(buf), "rm startup.%s", str);
  } else {
    // if third arg is passed in, and mode is OUTPUT, then set pin value
    if (getarg(0) == 3 && mode == OUTPUT) {
      uint8_t value = getarg(3);
      Scout.pinWrite(pin, value);
      snprintf(buf, sizeof(buf), "function startup.%s { pin.setmode(\"%s\",%d); pin.write(\"%s\",%d) }", str, str, mode, str, value);
    } else {
      snprintf(buf, sizeof(buf), "function startup.%s { pin.setmode(\"%s\",%d); }", str, str, mode);
    }
  }

  doCommand(buf);

  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }

  return true;
}

static numvar digitalPinReport(void) {
  speol(digitalPinReportHQ());
  return true;
}

static numvar analogPinReport(void) {
  speol(analogPinReportHQ());
  return true;
}

static int getPinFromArg(int arg) {
  if (isstringarg(arg)) {
    return Scout.getPinFromName((const char*)getstringarg(arg));
  } else if (Scout.isDigitalPin(getarg(arg)) || Scout.isAnalogPin(getarg(arg))) {
    return getarg(arg);
  } else {
    return -1;
  }
}

static bool checkArgs(uint8_t exactly, const __FlashStringHelper *errorMsg) {
  if (getarg(0) != exactly) {
      speol(errorMsg);
      return false;
  }
  return true;
}

static bool checkArgs(uint8_t min, uint8_t max, const __FlashStringHelper *errorMsg) {
  if (getarg(0) < min || getarg(0) > max) {
      speol(errorMsg);
      return false;
  }
  return true;
}

/****************************\
*     BACKPACK HANDLERS     *
\****************************/

static StringBuffer backpackReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d],[[", keyMap("backpacks", 0), keyMap("list", 0));
  /*
  for (uint8_t i=0; i<Backpacks::num_backpacks; ++i) {
    BackpackInfo &info = Backpacks::info[i];
    for (uint8_t j=0; j<sizeof(info.id); ++j) {
      // TODO this isn't correct, dunno what to do here
      snprintf(report+strlen(report), sizeof(report) - strlen(report), "%s%d",comma++?",":"",info.id.raw_bytes[j]);
    }
  }
  */
  report += "]]]";
  return Scout.handler.report(report);
}

static numvar backpackReport(void) {
  speol(backpackReportHQ());
  return 1;
}

static void printHexBuffer(Print &p, const uint8_t *buf, size_t len, const char *sep = NULL) {
  for (uint8_t i=0; i<len; ++i) {
    if (buf[i] < 0x10) {
      p.print('0');
    }
    p.print(buf[i], HEX);
    if (sep) {
      p.print(sep);
    }
  }
}

static numvar backpackList(void) {
  if (Backpacks::num_backpacks == 0) {
    speol(F("No backpacks found"));
  } else {
    for (uint8_t i=0; i<Backpacks::num_backpacks; ++i) {
      BackpackInfo &info = Backpacks::info[i];
      printHexBuffer(Serial, &i, 1);
      sp(F(": "));

      Pbbe::Header *h = info.getHeader();
      if (!h) {
        sp(F("Error parsing name"));
      } else {
        sp(h->backpack_name);
      }

      sp(F(" ("));
      printHexBuffer(Serial, info.id.raw_bytes, sizeof(info.id));
      speol(F(")"));
    }
  }
  return 0;
}

static numvar backpackEeprom(void) {
  numvar addr = getarg(1);
  if (addr < 0 || addr >= Backpacks::num_backpacks) {
    speol(F("Invalid backpack number"));
    return 0;
  }

  // Get EEPROM contents
  Pbbe::Eeprom *eep = Backpacks::info[addr].getEeprom();
  if (!eep) {
    speol(F("Failed to fetch EEPROM"));
    return 0;
  }

  // Print EEPROM over multiple lines
  size_t offset = 0;
  const uint8_t bytes_per_line = 8;
  while (offset < eep->size) {
    printHexBuffer(Serial, eep->raw + offset, min(bytes_per_line, eep->size - offset), " ");
    Serial.println();
    offset += bytes_per_line;
  }
  return 1;
}

static numvar backpackUpdateEeprom(void) {
  numvar addr = getarg(1);
  if (addr < 0 || addr >= Backpacks::num_backpacks) {
    speol(F("Invalid backpack number"));
    return 0;
  }

  numvar offset;
  const char *str;

  if (getarg(0) == 2) {
    offset = 0;
    str = (const char*)getstringarg(2);
  } else {
    offset = getarg(2);
    str = (const char*)getstringarg(3);
  }

  size_t length = strlen(str);
  uint8_t bytes[length / 2];
  PinoccioShell::parseHex(str, length, bytes);

  // Get the current EEPROM contents, but ignore any failure so we can
  // fix the EEPROM even when it has an invalid checksum (updateEeprom
  // handles a NULL as expected).
  BackpackInfo &info = Backpacks::info[addr];
  info.getEeprom();

  info.freeHeader();
  info.freeAllDescriptors();

  Pbbe::Eeprom *eep = Pbbe::updateEeprom(info.eep, offset, bytes, length / 2);
  if (!eep) {
    speol(F("Failed to update EEPROM"));
    return 0;
  }
  // Update the eep pointer, it might have been realloced
  info.eep = eep;

  if (!Pbbe::writeEeprom(Backpacks::pbbp, addr, info.eep)) {
    speol(F("Failed to write EEPROM"));
    return 0;
  }
  return 1;
}


static numvar backpackDetail(void) {
  numvar addr = getarg(1);
  if (addr < 0 || addr >= Backpacks::num_backpacks) {
    Serial.println(F("Invalid backpack number"));
    return 0;
  }
  Pbbe::Header *h = Backpacks::info[addr].getHeader();
  Pbbe::UniqueId &id = Backpacks::info[addr].id;

  // TODO: Convert these to sp()'s so we can see them in HQ, once sp/speol support the base argument
  Serial.print(F("Backpack name: "));
  Serial.println(h->backpack_name);

  Serial.print(F("Model number: 0x"));
  Serial.println(id.model, HEX); // TODO: zero pad

  Serial.print(F("Board revision: "));
  Pbbe::MajorMinor rev = Pbbe::extractMajorMinor(id.revision);
  Serial.print(rev.major);
  Serial.print(F("."));
  Serial.println(rev.minor);

  Serial.print(F("Serial number: 0x"));
  Serial.println(id.serial, HEX); // TODO: zero pad

  Serial.print(F("Backpack Bus Protocol version: "));
  Serial.print(id.protocol_version);
  Serial.println(F(".x")); // Only the major version is advertised

  Serial.print(F("Backpack Bus firmware version: "));
  Serial.println(h->firmware_version);

  Serial.print(F("EEPROM layout version: "));
  Serial.print(h->layout_version);
  Serial.println(F(".x")); // Only the major version is advertised

  Serial.print(F("EEPROM size: "));
  Serial.print(h->total_eeprom_size);
  Serial.println(F(" bytes"));

  Serial.print(F("EEPROM used: "));
  Serial.print(h->used_eeprom_size);
  Serial.println(F(" bytes"));
  return 1;
}

static numvar backpackResources(void) {
  numvar addr = getarg(1);
  if (addr < 0 || addr >= Backpacks::num_backpacks) {
    Serial.println(F("Invalid backpack number"));
    return 0;
  }

  Pbbe::DescriptorList *list = Backpacks::info[addr].getAllDescriptors();
  if (!list) {
    Serial.println(F("Failed to fetch or parse resource descriptors"));
    return 0;
  }
  for (uint8_t i = 0; i < list->num_descriptors; ++i) {
    Pbbe::DescriptorInfo &info = list->info[i];
    if (info.group) {
      Pbbe::GroupDescriptor& d = static_cast<Pbbe::GroupDescriptor&>(*info.group->parsed);
      Serial.print(d.name);
      Serial.print(".");
    }

    switch (info.type) {
      case Pbbe::DT_SPI_SLAVE: {
        Pbbe::SpiSlaveDescriptor& d = static_cast<Pbbe::SpiSlaveDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(F(": spi, ss = "));
        Serial.print(d.ss_pin.name());
        Serial.print(F(", max speed = "));
        if (d.speed.raw()) {
          Serial.print((float)d.speed, 2);
          Serial.print(F("Mhz"));
        } else {
          Serial.print(F("unknown"));
        }
        Serial.println();
        break;
      }
      case Pbbe::DT_UART: {
        Pbbe::UartDescriptor& d = static_cast<Pbbe::UartDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(F(": uart, tx = "));
        Serial.print(d.tx_pin.name());
        Serial.print(F(", rx = "));
        Serial.print(d.rx_pin.name());
        Serial.print(F(", speed = "));
        if (d.speed) {
          Serial.print(d.speed);
          Serial.print(F("bps"));
        } else {
          Serial.print(F("unknown"));
        }
        Serial.println();
        break;
      }
      case Pbbe::DT_IOPIN: {
        Pbbe::IoPinDescriptor& d = static_cast<Pbbe::IoPinDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(F(": gpio, pin = "));
        Serial.print(d.pin.name());
        Serial.println();
        break;
      }
      case Pbbe::DT_GROUP: {
  // Ignore
        break;
      }
      case Pbbe::DT_POWER_USAGE: {
        Pbbe::PowerUsageDescriptor& d = static_cast<Pbbe::PowerUsageDescriptor&>(*info.parsed);
        Serial.print(F("power: pin = "));
        Serial.print(d.power_pin.name());
        Serial.print(F(", minimum = "));
        if (d.minimum.raw()) {
          Serial.print((float)d.minimum, 2);
          Serial.print(F("uA"));
        } else {
          Serial.print(F("unknown"));
        }
        Serial.print(F(", typical = "));
        if (d.typical.raw()) {
          Serial.print((float)d.typical, 2);
          Serial.print(F("uA"));
        } else {
          Serial.print(F("unknown"));
        }
        Serial.print(F(", maximum = "));
        if (d.maximum.raw()) {
          Serial.print((float)d.maximum, 2);
          Serial.print(F("uA"));
        } else {
          Serial.print(F("unknown"));
        }
        Serial.println();
        break;
      }
      case Pbbe::DT_I2C_SLAVE: {
        Pbbe::I2cSlaveDescriptor& d = static_cast<Pbbe::I2cSlaveDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(F(": i2c, address = "));
        Serial.print(d.addr);
        Serial.print(F(", max speed = "));
        Serial.print(d.speed);
        Serial.print(F("kbps"));
        Serial.println();
        break;
      }
      case Pbbe::DT_DATA: {
        Pbbe::DataDescriptor& d = static_cast<Pbbe::DataDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(F(": data, length = "));
        Serial.print(d.length);
        Serial.print(F(", content = "));
        printHexBuffer(Serial, d.data, d.length);
        Serial.println();
        break;
      }
      default: {
  // Should not occur
        break;
      }
    }
  }

  return 1;
}

/****************************\
 *   SCOUT REPORT HANDLERS  *
\****************************/

static StringBuffer scoutReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d,%d,%d,%d,%d,%d,%d],[%s,%d,%d,%d,%ld,\"%s\",%ld,\"%s\"]]",
          keyMap("scout", 0),
          keyMap("lead", 0),
          keyMap("version", 0),
          keyMap("hardware", 0),
          keyMap("family", 0),
          keyMap("serial", 0),
          keyMap("sketch", 0),
          keyMap("build", 0),
          keyMap("revision", 0),
          Scout.isLeadScout() ? "true" : "false",
          (int)Scout.getEEPROMVersion(),
          (int)Scout.getHwVersion(),
          Scout.getHwFamily(),
          Scout.getHwSerial(),
          Scout.getSketchName(),
          Scout.getSketchBuild(),
          Scout.getSketchRevision());
  return Scout.handler.report(report);
}

static numvar scoutReport(void) {
  speol(scoutReportHQ());
  return 1;
}

static numvar isScoutLeadScout(void) {
  return Scout.isLeadScout();
}

static numvar setHQToken(void) {
  if (!checkArgs(1, F("usage: hq.settoken(\"token\""))) {
    return 0;
  }
  Scout.setHQToken((const char *)getstringarg(1));
  return 1;
}

static numvar getHQToken(void) {
  char token[33];
  Scout.getHQToken((char *)token);
  token[32] = 0;
  speol(token);
  return keyMap(token, millis());
}

static void delayTimerHandler(SYS_Timer_t *timer) {
  doCommand(((char*)timer) + (sizeof(struct SYS_Timer_t)));
  free(timer);
}

void PinoccioShell::delay(uint32_t at, char *command) {
  size_t clen = strlen(command) + 1;
  // allocate space for the command after the timer pointer
  SYS_Timer_t *delayTimer = (SYS_Timer_t *)malloc(sizeof(struct SYS_Timer_t) + clen);
  memset(delayTimer,0,sizeof(struct SYS_Timer_t));
  memcpy(((char*)delayTimer) + sizeof(struct SYS_Timer_t), command, clen);
  // init timer
  delayTimer->mode = SYS_TIMER_INTERVAL_MODE;
  delayTimer->handler = delayTimerHandler;
  delayTimer->interval = at;
  SYS_TimerStart(delayTimer);
}

static numvar scoutDelay(void) {
  char *str;
  int i, args = getarg(0);
  uint32_t accum = 0;
  if (!args || args % 2) {
    speol("usage: scout.delay(ms,\"function\",...)");
    return 0;
  }
  for (i=1; i<args; i+=2) {
    // copy at the end the command string
    if (isstringarg(i+1)) {
      str = (char *)getarg(i+1);
    } else {
      str = (char *)keyGet(getarg(i+1));
    }
    accum += (uint32_t)getarg(i);
    Shell.delay(accum, str);
  }
  return 1;
}

static numvar memoryReport(void) {
  StringBuffer report(100);
  int freeMem = getFreeMemory();
  report.appendSprintf("[%d,[%d,%d,%d],[%d,%d,%d]]",
          keyMap("memory", 0),
          keyMap("used", 0),
          keyMap("free", 0),
          keyMap("large", 0),
          getMemoryUsed(),
          freeMem,
          getLargestAvailableMemoryBlock());
  speol(Scout.handler.report(report));
  return freeMem;
}

static numvar daisyWipe(void) {
  bool ret = true;

  if (Scout.factoryReset() == false) {
    speol(F("Factory reset requested. Send command again to confirm."));
    return false;
  } else {
    speol(F("Ok, terminating. Goodbye Dave."));
  }

  char report[32];
  snprintf(report, sizeof(report),"[%d,[%d],[\"bye\"]]",keyMap("daisy",0),keyMap("dave",0));
  Scout.handler.report(report);

  if (Scout.isLeadScout()) {
    if (!Scout.wifi.runDirectCommand(Serial, "AT&F")) {
       sp(F("Error: Wi-Fi direct command failed"));
       ret = false;
    }
    if (!Scout.wifi.runDirectCommand(Serial, "AT&W0")) {
       sp(F("Error: Wi-Fi direct command failed"));
       ret = false;
    }
  }

  if (ret == true) {
    Scout.meshResetSecurityKey();
    Scout.meshSetRadio(0, 0x0000);
    Scout.resetHQToken();

    // so long, and thanks for all the fish!
    doCommand("rm *");
    doCommand("scout.boot");
  }
  return 1;
}

static numvar boot(void) {
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
  return 1;
}

static numvar otaBoot(void) {
  Scout.setOTAFlag();
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
  return 1;
}


/****************************\
 *        HQ HANDLERS       *
\****************************/

static numvar hqVerbose(void) {
  Scout.handler.setVerbose(getarg(1));
  return 1;
}

static numvar hqPrint(void) {
  if (!checkArgs(1, F("usage: hq.print(\"string\""))) {
    return 0;
  }
  Scout.handler.announce(0, arg2array(0));
  return true;
}

static numvar hqReport(void) {
  if (!checkArgs(2, F("usage: hq.report(\"reportname\", \"value\")"))) {
    return 0;
  }
  const char *name = (isstringarg(1))?(const char*)getarg(1):keyGet(getarg(1));
  if (!name || strlen(name) == 0) {
    speol("report name must be the first argument");
    return false;
  }
  char *args = strdup(arg2array(-1).c_str());
  if (strlen(args)+strlen(name) > 80) {
    free(args);
    speol("report too large");
    return false;
  }
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[\"%s\",%s]]",
          keyMap("custom", 0),
          keyMap("name", 0),
          keyMap("custom", 0),
          name,
          args);
  free(args);
  speol(Scout.handler.report(report));
  return true;
}

static numvar hqBridge(void){
  Scout.handler.setBridgeMode(getarg(1));
  isBridgeMode = getarg(1);
  if (isBridgeMode) {
    speol("on");
  } else {
    speol("off");
  }
  return 1;
}

static numvar hqBridgeCommand(void){
  if (!checkArgs(3, F("usage: hq.bridge.command(\"command\", scout id, reply id)"))) {
    return 0;
  }

  Scout.handler.sendCommand((char *)getstringarg(1), getarg(2), getarg(3));
  return 1;
}





/****************************\
 *      EVENT HANDLERS      *
\****************************/

static numvar startStateChangeEvents(void) {
  Scout.startDigitalStateChangeEvents();
  Scout.startAnalogStateChangeEvents();
  Scout.startPeripheralStateChangeEvents();
  return 1;
}

static numvar stopStateChangeEvents(void) {
  Scout.stopDigitalStateChangeEvents();
  Scout.stopAnalogStateChangeEvents();
  Scout.stopPeripheralStateChangeEvents();
  return 1;
}

static numvar setEventCycle(void) {
  Scout.setStateChangeEventCycle(getarg(1), getarg(2), getarg(3));
  return 1;
}

static numvar setEventVerbose(void) {
  Scout.eventVerboseOutput = getarg(1);
  return 1;
}


/****************************\
 *   SCOUT.WIFI.HANDLERS    *
\****************************/

static StringBuffer wifiReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[%s,%s]]",
          keyMap("wifi", 0),
          keyMap("connected", 0),
          keyMap("hq", 0),
          Scout.wifi.isAPConnected() ? "true" : "false",
          Scout.wifi.isHQConnected() ? "true" : "false");
  return Scout.handler.report(report);
}

static numvar wifiReport(void) {
  speol(wifiReportHQ());
  return 1;
}

static numvar wifiStatus(void) {
  if (getarg(0) > 0 && getarg(1) == 1) {
    Scout.wifi.printProfiles(Serial);
  } else {
    Scout.wifi.printFirmwareVersions(Serial);
    Scout.wifi.printCurrentNetworkStatus(Serial);
  }
  return 1;
}

static numvar wifiList(void) {
  if (!Scout.wifi.printAPs(Serial)) {
    speol(F("Error: Scan failed"));
    return 0;
  }
  return 1;
}

static numvar wifiConfig(void) {
  if (!checkArgs(2, F("usage: wifi.config(\"wifiAPName\", \"wifiAPPassword\")"))) {
    return 0;
  }

  if (!Scout.wifi.wifiConfig((const char *)getstringarg(1), (const char *)getstringarg(2))) {
    speol(F("Error: saving Scout.wifi.configuration data failed"));
  }
  return 1;
}

static numvar wifiDhcp(void) {
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!Scout.wifi.wifiDhcp(host)) {
    speol(F("Error: saving Scout.wifi.configuration data failed"));
  }
  return 1;
}

static numvar wifiStatic(void) {
  if (!checkArgs(4, F("usage: wifi.static(\"ip\", \"netmask\", \"gateway\", \"dns\")"))) {
    return 0;
  }

  IPAddress ip, nm, gw, dns;

  if (!GSCore::parseIpAddress(&ip, (const char *)getstringarg(1))) {
    speol(F("Error: Invalid IP address"));
    return 0;
  }

  if (!GSCore::parseIpAddress(&nm, (const char *)getstringarg(2))) {
    speol(F("Error: Invalid netmask"));
    return 0;
  }

  if (!GSCore::parseIpAddress(&gw, (const char *)getstringarg(3))) {
    speol(F("Error: Invalid gateway"));
    return 0;
  }

  if (!GSCore::parseIpAddress(&dns, (const char *)getstringarg(3))) {
    speol(F("Error: Invalid dns server"));
    return 0;
  }

  if (!Scout.wifi.wifiStatic(ip, nm, gw, dns)) {
    speol(F("Error: saving Scout.wifi.configuration data failed"));
    return 0;
  }
  return 1;
}

static numvar wifiDisassociate(void) {
  Scout.wifi.disassociate();
  return 1;
}

static numvar wifiReassociate(void) {
  // This restart the NCM
  return Scout.wifi.autoConnectHq();
}

static numvar wifiCommand(void) {
  if (!checkArgs(1, F("usage: wifi.command(\"command\")"))) {
    return 0;
  }
  if (!Scout.wifi.runDirectCommand(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi direct command failed"));
  }
  return 1;
}

static numvar wifiPing(void) {
  if (!checkArgs(1, F("usage: wifi.ping(\"hostname\")"))) {
    return 0;
  }
  if (!Scout.wifi.ping(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi ping command failed"));
  }
  return 1;
}

static numvar wifiDNSLookup(void) {
  if (!checkArgs(1, F("usage: wifi.dnslookup(\"hostname\")"))) {
    return 0;
  }
  if (!Scout.wifi.dnsLookup(Serial, (const char *)getstringarg(1))) {
     speol(F("Error: Wi-Fi DNS lookup command failed"));
  }
  return 1;
}

static numvar wifiGetTime(void) {
  if (!Scout.wifi.printTime(Serial)) {
     speol(F("Error: Wi-Fi NTP time lookup command failed"));
  }
  return 1;
}

static numvar wifiSleep(void) {
  if (!Scout.wifi.goToSleep()) {
     speol(F("Error: Wi-Fi sleep command failed"));
  }
  return 1;
}

static numvar wifiWakeup(void) {
  if (!Scout.wifi.wakeUp()) {
     speol(F("Error: Wi-Fi wakeup command failed"));
  }
  return 1;
}

static numvar wifiVerbose(void) {
  // TODO
  return 1;
}

static numvar wifiStats(void) {
  sp(F("Number of connections to AP since boot: "));
  speol(Scout.wifi.apConnCount);
  sp(F("Number of connections to HQ since boot: "));
  speol(Scout.wifi.hqConnCount);
}


/****************************\
 *     HELPER FUNCTIONS     *
\****************************/

static bool receiveMessage(NWK_DataInd_t *ind) {
  char buf[64];
  char *data = (char*)ind->data;
  int keys[10];

  if (isMeshVerbose) {
    Serial.print(F("Received message of "));
    Serial.print(data);
    Serial.print(F(" from "));
    Serial.print(ind->srcAddr, DEC);
    Serial.print(F(" lqi "));
    Serial.print(ind->lqi, DEC);
    Serial.print(F(" rssi "));
    Serial.println(abs(ind->rssi), DEC);
  }
  lastMeshRssi = abs(ind->rssi);
  lastMeshLqi = ind->lqi;
  NWK_SetAckControl(abs(ind->rssi));

  if (strlen(data) <3 || data[0] != '[') {
    return false;
  }

  // parse the array payload into keys, [1, "foo", "bar"]
  keyLoad(data, keys, millis());
  uint32_t time = millis();

  snprintf(buf, sizeof(buf),"on.message.scout");
  if (find_user_function(buf) || findscript(buf)) {
    snprintf(buf, sizeof(buf), "on.message.scout(%d", ind->srcAddr);
    for (int i=2; i<=keys[0]; i++) {
      snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ",%d", keys[i]);
    }
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ")");
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("on.message.scout event handler took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
  return true;
}

static void sendMessage(int address, const String &data) {
  if (sendDataReqBusy) {
    return;
  }

  sendDataReq.dstAddr = address;
  sendDataReq.dstEndpoint = 1;
  sendDataReq.srcEndpoint = 1;
  sendDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  sendDataReq.data = (uint8_t*)strdup(data.c_str());
  sendDataReq.size = data.length() + 1;
  sendDataReq.confirm = sendConfirm;
  NWK_DataReq(&sendDataReq);

  sendDataReqBusy = true;

  if (isMeshVerbose) {
    Serial.print(F("Sent message to Scout "));
    Serial.print(address);
    Serial.print(F(": "));
    Serial.println(data);
  }
}

static void sendConfirm(NWK_DataReq_t *req) {
   sendDataReqBusy = false;
   free(req->data);

   if (isMeshVerbose) {
    if (req->status == NWK_SUCCESS_STATUS) {
      Serial.print(F("-  Message successfully sent to Scout "));
      Serial.print(req->dstAddr);
      if (req->control) {
        Serial.print(F(" (Confirmed with control byte: "));
        Serial.print(req->control);
        Serial.print(F(")"));
      }
      Serial.println();
    } else {
      Serial.print(F("Error: "));
      switch (req->status) {
        case NWK_OUT_OF_MEMORY_STATUS:
          Serial.print(F("Out of memory: "));
          break;
        case NWK_NO_ACK_STATUS:
        case NWK_PHY_NO_ACK_STATUS:
          Serial.print(F("No acknowledgement received: "));
          break;
        case NWK_NO_ROUTE_STATUS:
          Serial.print(F("No route to destination: "));
          break;
        case NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS:
          Serial.print(F("Physical channel access failure: "));
          break;
        default:
          Serial.print(F("unknown failure: "));
      }
      Serial.print(F("("));
      Serial.print(req->status, HEX);
      Serial.println(F(")"));
    }
  }
  lastMeshRssi = req->control;

  // run the Bitlash callback ack function
  char buf[32];
  uint32_t time = millis();

  snprintf(buf, sizeof(buf),"on.message.signal");
  if (find_user_function(buf) || findscript(buf)) {
    snprintf(buf, sizeof(buf), "on.message.signal(%d, %d)", req->dstAddr, (req->status == NWK_SUCCESS_STATUS) ? req->control : 0);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("on.message.signal event handler took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
}


/****************************\
 *      EVENT HANDLERS      *
\****************************/

static void digitalPinEventHandler(uint8_t pin, int8_t value, int8_t mode) {
  uint32_t time = millis();
  char buf[16];

  digitalPinReportHQ();

  snprintf(buf, sizeof(buf), "on.d%d", pin);
  if (find_user_function(buf) || findscript(buf)) {
    snprintf(buf, sizeof(buf), "on.d%d(%d,%d)", pin, value, mode);
    doCommand(buf);
  }

  // simplified button trigger
  if (mode == INPUT_PULLUP || mode == INPUT) {
    if (value == 0) {
      snprintf(buf, sizeof(buf), "on.d%d.low", pin);
    } else {
      snprintf(buf, sizeof(buf), "on.d%d.high", pin);
    }
    if (find_user_function(buf) || findscript(buf)) {
      doCommand(buf);
    }
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("Digital pin event handlers took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
}

static void analogPinEventHandler(uint8_t pin, int16_t value, int8_t mode) {
  uint32_t time = millis();
  char buf[16];

  analogPinReportHQ();

  snprintf(buf, sizeof(buf),"on.a%d", pin);
  if (find_user_function(buf) || findscript(buf)) {
    snprintf(buf, sizeof(buf), "on.a%d(%d, %d)", pin, value, mode);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("Analog pin event handlers took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
}

static void batteryPercentageEventHandler(uint8_t value) {
  uint32_t time = millis();
  char buf[24];
  char *func = "on.battery.level";

  powerReportHQ();

  if (find_user_function(func) || findscript(func)) {
    snprintf(buf, sizeof(buf), "%s(%d)", func, value);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("on.battery.level event handler took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
}

static void batteryChargingEventHandler(uint8_t value) {
  uint32_t time = millis();
  char buf[28];
  char *func = "on.battery.charging";

  powerReportHQ();

  if (find_user_function(func) || findscript(func)) {
    snprintf(buf, sizeof(buf), "%s(%d)", func, value);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("on.battery.charging event handler took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
}

static void temperatureEventHandler(int8_t tempC, int8_t tempF) {
  uint32_t time = millis();
  char buf[28];
  char *func = "on.temperature";

  tempReportHQ();

  if (find_user_function(func) || findscript(func)) {
    snprintf(buf, sizeof(buf), "%s(%d, %d)", func, tempC, tempF);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print(F("on.temperature event handler took "));
    Serial.print(millis() - time);
    Serial.println(F("ms"));
  }
}

static void ledEventHandler(uint8_t redValue, uint8_t greenValue, uint8_t blueValue) {
  ledReportHQ();
}
