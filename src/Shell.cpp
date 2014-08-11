/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include "stdarg.h"
#include "Shell.h"
#include "Scout.h"
#include "SleepHandler.h"
#include "backpacks/Backpacks.h"
#include "SleepHandler.h"
#include "bitlash.h"
#include "src/bitlash.h"
#include "util/String.h"
#include "util/PrintToString.h"
extern "C" {
#include "key/key.h"
#include "util/memdebug.h"
}

static numvar pinoccioBanner(void);

static numvar getTemperatureC(void);
static numvar getTemperatureF(void);
static numvar temperatureReport(void);
static numvar setTemperatureOffset(void);
static numvar temperatureCalibrate(void);
static numvar getRandomNumber(void);

static numvar allReport(void);
static numvar allVerbose(void);

static numvar uptimeAwakeMicros(void);
static numvar uptimeAwakeSeconds(void);
static numvar uptimeSleepingMicros(void);
static numvar uptimeSleepingSeconds(void);
static numvar uptimeMicros(void);
static numvar uptimeSeconds(void);
static numvar uptimeDays(void);
static numvar uptimePrint(void);
static numvar uptimeReport(void);
static numvar uptimeStatus(void);
static numvar getLastResetCause(void);

static numvar isBatteryCharging(void);
static numvar isBatteryConnected(void);
static numvar getBatteryPercentage(void);
static numvar getBatteryVoltage(void);
static numvar enableBackpackVcc(void);
static numvar disableBackpackVcc(void);
static numvar isBackpackVccEnabled(void);
static numvar powerSleep(void);
static numvar powerReport(void);
static numvar powerWakeupPin(void);

static numvar ledBlink(void);
static numvar ledOff(void);
static numvar ledRed(void);
static numvar ledGreen(void);
static numvar ledBlue(void);
static numvar ledCyan(void);
static numvar ledPurple(void);
static numvar ledBeccaPurple(void);
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
static numvar pinConstDisconnected(void);
static numvar pinConstDisabled(void);
static numvar pinConstInput(void);
static numvar pinConstOutput(void);
static numvar pinConstInputPullup(void);
static numvar pinConstPWM(void);
static numvar pinMakeInput(void);
static numvar pinMakeOutput(void);
static numvar pinMakePWM(void);
static numvar pinMakeDisconnected(void);
static numvar pinDisable(void);
static numvar pinSetMode(void);
static numvar pinRead(void);
static numvar pinWrite(void);
static numvar pinSave(void);
static numvar pinStatus(void);
static numvar pinNumber(void);
static numvar pinOthersDisconnected(void);
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

static numvar moduleList(void);
static numvar moduleLoad(void);
static numvar moduleLoaded(void);

static numvar hqVerbose(void);
static numvar hqPrint(void);
static numvar hqReport(void);
static numvar hqBridge(void);

static numvar startStateChangeEvents(void);
static numvar stopStateChangeEvents(void);
static numvar setEventCycle(void);
static numvar setEventVerbose(void);

static numvar keySer(void);
static numvar keyFree(void);
static numvar keyPrint(void);
static numvar keyNumber(void);
static numvar keySave(void);

static int8_t getPinFromArg(uint8_t arg);

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

static void digitalPinEventHandler(uint8_t pin, int16_t value, int8_t mode);
static void analogPinEventHandler(uint8_t pin, int16_t value, int8_t mode);
static void batteryPercentageEventHandler(uint8_t value);
static void batteryChargingEventHandler(uint8_t value);
static void temperatureEventHandler(int8_t tempC, int8_t tempF);
static void ledEventHandler(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);

static void printSpaces(int8_t number) {
  while (number-- > 0)
    Serial.write(' ');
}

PinoccioShell Shell;
StringBuffer bitlashOutput;

PinoccioShell::PinoccioShell() {
  isShellEnabled = true;
}

PinoccioShell::~PinoccioShell() { }

void PinoccioShell::setup() {
  keyInit();

  addFunction("power.ischarging", isBatteryCharging);
  addFunction("power.hasbattery", isBatteryConnected);
  addFunction("power.percent", getBatteryPercentage);
  addFunction("power.voltage", getBatteryVoltage);
  addFunction("power.enablevcc", enableBackpackVcc);
  addFunction("power.disablevcc", disableBackpackVcc);
  addFunction("power.isvccenabled", isBackpackVccEnabled);
  addFunction("power.sleep", powerSleep);
  addFunction("power.report", powerReport);
  addFunction("power.wakeup.pin", powerWakeupPin);

  addFunction("mesh.config", meshConfig);
  addFunction("mesh.setpower", meshSetPower);
  addFunction("mesh.setdatarate", meshSetDataRate);
  addFunction("mesh.setkey", meshSetKey);
  addFunction("mesh.getkey", meshGetKey);
  addFunction("mesh.resetkey", meshResetKey);
  addFunction("mesh.joingroup", meshJoinGroup);
  addFunction("mesh.leavegroup", meshLeaveGroup);
  addFunction("mesh.ingroup", meshIsInGroup);
  addFunction("mesh.verbose", meshVerbose);
  addFunction("mesh.report", meshReport);
  addFunction("mesh.routing", meshRouting);
  addFunction("mesh.signal", meshSignal);
  addFunction("mesh.loss", meshLoss);

  addFunction("message.scout", messageScout);
  addFunction("message.group", messageGroup);

  addFunction("temperature.c", getTemperatureC);
  addFunction("temperature.f", getTemperatureF);
  addFunction("temperature.report", temperatureReport);
  addFunction("temperature.setoffset", setTemperatureOffset);
  addFunction("temperature.calibrate", temperatureCalibrate);
  addFunction("randomnumber", getRandomNumber);
  addFunction("memory.report", memoryReport);

  addFunction("report", allReport);
  addFunction("verbose", allVerbose);

  addFunction("uptime", uptimePrint);
  addFunction("uptime.awake.micros", uptimeAwakeMicros);
  addFunction("uptime.awake.seconds", uptimeAwakeSeconds);
  addFunction("uptime.sleeping.micros", uptimeSleepingMicros);
  addFunction("uptime.sleeping.seconds", uptimeSleepingSeconds);
  addFunction("uptime.seconds", uptimeSeconds);
  addFunction("uptime.days", uptimeDays);
  addFunction("uptime.micros", uptimeMicros);
  addFunction("uptime.report", uptimeReport);
  addFunction("uptime.getlastreset", getLastResetCause);
  addFunction("uptime.status", uptimeStatus);

  addFunction("led.on", ledTorch); // alias
  addFunction("led.off", ledOff);
  addFunction("led.red", ledRed);
  addFunction("led.green", ledGreen);
  addFunction("led.blue", ledBlue);
  addFunction("led.cyan", ledCyan);
  addFunction("led.purple", ledPurple);
  addFunction("led.beccapurple", ledBeccaPurple);
  addFunction("led.magenta", ledMagenta);
  addFunction("led.yellow", ledYellow);
  addFunction("led.orange", ledOrange);
  addFunction("led.white", ledWhite);
  addFunction("led.torch", ledTorch);
  addFunction("led.blink", ledBlink);
  addFunction("led.sethex", ledSetHex);
  addFunction("led.gethex", ledGetHex);
  addFunction("led.setrgb", ledSetRgb);
  addFunction("led.isoff", ledIsOff);
  addFunction("led.savetorch", ledSaveTorch);
  addFunction("led.report", ledReport);

  addFunction("high", pinConstHigh);
  addFunction("low", pinConstLow);
  addFunction("disconnected", pinConstDisconnected);
  addFunction("disabled", pinConstDisabled);
  addFunction("input", pinConstInput);
  addFunction("output", pinConstOutput);
  addFunction("input_pullup", pinConstInputPullup);
  addFunction("pwm", pinConstPWM);

  addFunction("pin.makeinput", pinMakeInput);
  addFunction("pin.makeoutput", pinMakeOutput);
  addFunction("pin.makepwm", pinMakePWM);
  addFunction("pin.makedisconnected", pinMakeDisconnected);
  addFunction("pin.disable", pinDisable);
  addFunction("pin.setmode", pinSetMode);
  addFunction("pin.read", pinRead);
  addFunction("pin.write", pinWrite);
  addFunction("pin.save", pinSave);
  addFunction("pin.status", pinStatus);
  addFunction("pin.number", pinNumber);
  addFunction("pin.othersdisconnected", pinOthersDisconnected);
  addFunction("pin.report.digital", digitalPinReport);
  addFunction("pin.report.analog", analogPinReport);

  addFunction("backpack.report", backpackReport);
  addFunction("backpack.list", backpackList);
  addFunction("backpack.eeprom", backpackEeprom);
  addFunction("backpack.eeprom.update", backpackUpdateEeprom);
  addFunction("backpack.detail", backpackDetail);
  addFunction("backpack.resources", backpackResources);

  addFunction("scout.report", scoutReport);
  addFunction("scout.isleadscout", isScoutLeadScout);
  addFunction("scout.delay", scoutDelay);
  addFunction("scout.daisy", daisyWipe);
  addFunction("scout.boot", boot);
  addFunction("scout.otaboot", otaBoot);

  addFunction("module.list", moduleList);
  addFunction("module.load", moduleLoad);
  addFunction("module.loaded", moduleLoaded);

  addFunction("hq.settoken", setHQToken);
  addFunction("hq.gettoken", getHQToken);
  addFunction("hq.verbose", hqVerbose);
  addFunction("hq.print", hqPrint);
  addFunction("hq.report", hqReport);
  addFunction("hq.bridge", hqBridge);

  addFunction("events.start", startStateChangeEvents);
  addFunction("events.stop", stopStateChangeEvents);
  addFunction("events.setcycle", setEventCycle);
  addFunction("events.verbose", setEventVerbose);

  addFunction("key", keySer);
  addFunction("key.free", keyFree);
  addFunction("key.print", keyPrint);
  addFunction("key.number", keyNumber);
  addFunction("key.save", keySave);

  // set up event handlers
  Scout.digitalPinEventHandler = digitalPinEventHandler;
  Scout.analogPinEventHandler = analogPinEventHandler;
  Scout.batteryPercentageEventHandler = batteryPercentageEventHandler;
  Scout.batteryChargingEventHandler = batteryChargingEventHandler;
  Scout.temperatureEventHandler = temperatureEventHandler;
  Led.ledEventHandler = ledEventHandler;

  if (isShellEnabled) {
    startShell();
  }

  Scout.meshListen(1, receiveMessage);

  if (!Scout.isLeadScout()) {
    Shell.allReportHQ(); // lead scout reports on hq connect
  }
  
}

void PinoccioShell::addFunction(const char *name, numvar (*func)(void)) {
  addBitlashFunction(name, (bitlash_function)func);
}

// update a memory cache of which functions are defined
StringBuffer customScripts;
void PinoccioShell::refresh(void)
{
  bitlashOutput = "";
  doCommand("ls");
  customScripts = bitlashOutput;
  bitlashOutput = (char*)NULL;

  // parse and condense the "ls" bitlash format of "function name {...}\n" into just "name "
  int nl, sp;
  while((nl = customScripts.indexOf('\n')) >= 0)
  {
    if(customScripts.startsWith("function ") && (sp = customScripts.indexOf(' ',9)) < nl)
    {
      customScripts += customScripts.substring(9,sp+1);
    }
    customScripts = customScripts.substring(nl+1);
  }
}

// just a safe wrapper around bitlash checks
bool PinoccioShell::defined(const char *cmd)
{
  if(!cmd) return false;
  if(find_user_function((char*)cmd)) return true;
//  if(findKey(cmd) >= 0) return true; // don't use findscript(), it's not re-entrant safe
  int at = customScripts.indexOf(cmd);
  if(at >= 0 && customScripts.charAt(at+strlen(cmd)) == ' ') return true;
  return false;
}

static bool isMeshVerbose;
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

// only print to serial if/when we are not handling bitlash
numvar PinoccioShell::eval(const char *str) {
  return eval(str, NULL);
}
numvar PinoccioShell::eval(const char *str, StringBuffer *result) {
  numvar ret;
  bitlashOutput = "";
  ret = doCommand((char*)str);
  if(result) *result += bitlashOutput;
  bitlashOutput = (char*)NULL;
  refresh();
  return ret;
}
StringBuffer execOut;
const char *PinoccioShell::exec(const char *str) {
  numvar ret;
  execOut = "";
  ret = eval(str,&execOut);
  // nice convenience
  if(execOut == "") execOut.appendSprintf("%ld",ret);
  return execOut.c_str();
}
numvar PinoccioShell::command(const char *cmd, ...) {
  int i, n;
  StringBuffer str;
  va_list vl;
  if(!defined(cmd)) return 0;
  va_start(vl,n);
  str = cmd;
  str += "(";
  for (i=0;i<n;i++)
  {
    if(i) str += ",";
    str += "\"";
    str += strlen(va_arg(vl, char*))+3;
    str += "\"";
  }
  str += ")";
  return eval(str.c_str());
}

StringBuffer serialWaiting;
void PinoccioShell::prompt(void) {
  Serial.print(F("> "));
  // no longer blocking any other output
  outWait = false;
  // dump and clear any waiting output
  Serial.print(serialWaiting.c_str());
  serialWaiting = (char*)NULL;
}

// only print to serial if/when we are not handling bitlash
void PinoccioShell::print(const char *str) {
  if(outWait) {
    serialWaiting += str;
  } else {
    Serial.print(str);
  }
}

static StringBuffer serialIncoming;
static char lastc = 0;

StringBuffer serialOutgoing;
void PinoccioShell::loop() {
  if (isShellEnabled) {
    while (Serial.available()) {
      char c = Serial.read();

      if (c == '\n' && lastc == '\r') {
        // Ignore the \n in \r\n to prevent interpreting \r\n as two
        // newlines. Note that we cannot just ignore newlines when the
        // buffer is empty, since that doesn't allow forcing a new
        // prompt by sending a newline (when the terminal is messed up
        // by debug output for example).
      } if (c == '\r' || c == '\n') {
        Serial.println();
        eval(serialIncoming.c_str(), &serialOutgoing);
        Serial.print(serialOutgoing.c_str());
        serialIncoming = serialOutgoing = (char*)NULL;
        prompt();
      } else {
        outWait = true; // reading stuff, don't print anything else out
        Serial.write(c); // echo everything back
        serialIncoming += c;
      }
      lastc = c;
    }
    // bitlash loop
    runBackgroundTasks();
    Serial.print(bitlashOutput.c_str());
    bitlashOutput = (char*)NULL;
  }
  keyLoop(millis());
}

void PinoccioShell::startShell() {
  char buf[32];
  uint8_t i;

  isShellEnabled = true;

  // init bitlash internals, don't use initBitlash so we do our own serial
  setOutputHandler(&printToString<&bitlashOutput>);
  initTaskList();
  vinit();
  
  // init our defined cache and start up
  Shell.refresh();
  pinoccioBanner();

  snprintf(buf, sizeof(buf), "startup", i);
  if (Shell.defined(buf)) {
    doCommand(buf);
  }

  for (i='a'; i<'z'; i++) {
    snprintf(buf, sizeof(buf), "startup.%c", i);
    if (Shell.defined(buf)) {
      doCommand(buf);
    }
  }

  for (i=0; i<NUM_DIGITAL_PINS; ++i) {
    strlcpy(buf, "startup.", sizeof(buf));
    strlcat_P(buf, (const char*)Scout.getNameForPin(i), sizeof(buf));

    if (Shell.defined(buf)) {
      doCommand(buf);
    }
  }

  // print anything that may have shown up from startup
  Serial.print(bitlashOutput.c_str());
  bitlashOutput = (char*)NULL;
  prompt();
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
  report.appendSprintf("[%d,[%d,%d,%d],[%d,%d,%d]]",
          keyMap("temp", 0),
          keyMap("c", 0),
          keyMap("f", 0),
          keyMap("offset", 0),
          Scout.getTemperatureC(),
          Scout.getTemperatureF(),
          Scout.getTemperatureOffset());
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

static numvar setTemperatureOffset(void) {
  if (!checkArgs(1, F("usage: temperature.setoffset(value)"))) {
    return 0;
  }
  Scout.setTemperatureOffset(getarg(1));
  return 1;
}

static numvar temperatureCalibrate(void) {
  if (!checkArgs(1, F("usage: temperature.setoffset(value)"))) {
    return 0;
  }
  Scout.setTemperatureOffset(getarg(1) - Scout.getTemperatureC());
  return 1;
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
          keyMap("total", 0),
          keyMap("sleep", 0),
          keyMap("random", 0),
          keyMap("reset", 0),
          SleepHandler::uptime().seconds,
          SleepHandler::sleeptime().seconds,
          (int)random());

  report.appendJsonString(reset, true);
  report += "]]";
  return Scout.handler.report(report);
}

static numvar uptimeAwakeMicros(void) {
  return SleepHandler::waketime().us;
}

static numvar uptimeAwakeSeconds(void) {
  return SleepHandler::waketime().seconds;
}

static numvar uptimeSleepingMicros(void) {
  return SleepHandler::sleeptime().us;
}

static numvar uptimeSleepingSeconds(void) {
  return SleepHandler::sleeptime().seconds;
}

static numvar uptimeMicros(void) {
  return SleepHandler::uptime().us;
}

static numvar uptimePrint(void) {
  speol(SleepHandler::uptime().seconds);
  return SleepHandler::uptime().seconds;
}

static numvar uptimeSeconds(void) {
  return SleepHandler::uptime().seconds;
}

static numvar uptimeDays(void) {
  return SleepHandler::uptime().seconds/86400;
}

static numvar uptimeReport(void) {
  speol(uptimeReportHQ());
  return true;
}

static void appendTime(StringBuffer &b, Duration d) {
  unsigned days = d.seconds / 3600 / 24;
  unsigned hours = d.seconds / 3600 % 24;
  unsigned minutes = d.seconds / 60 % 60;
  unsigned seconds = d.seconds % 60;

  b.appendSprintf("%u days, %u hours, %u minutes, %d.%06lu seconds",
                  days, hours, minutes, seconds, d.us);
}

static numvar uptimeStatus(void) {
  StringBuffer out(100);
  out = F("Total: ");
  appendTime(out, SleepHandler::uptime());
  speol(out.c_str());

  out = F("Awake: ");
  appendTime(out, SleepHandler::waketime());
  speol(out.c_str());

  out = F("Asleep: ");
  appendTime(out, SleepHandler::sleeptime());
  speol(out.c_str());

  return true;
}

/****************************\
*        KEY HANDLERS        *
\****************************/

static numvar keySer(void) {
  if (!checkArgs(1, 2, F("usage: key(\"string\" [, temp_flag])"))) {
    return 0;
  }
  unsigned long at = 0;
  static char num[8];
  // if the temp flag was passed
  if(getarg(0) > 1) {
    at = getarg(2);
  }
  if (isstringarg(1)) {
    return keyMap((char*)getstringarg(1), at);
  }
  snprintf(num, 8, "%ld", getarg(1));
  return keyMap(num, at);
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

static numvar powerSleep(void) {
  if (!getarg(0) || getarg(0) > 2) {
    speol("usage: power.sleep(ms, [\"function\"])");
    return 0;
  }

  const char *func = NULL;
  if (getarg(0) > 1) {
    if (isstringarg(2))
      func = (char*)getstringarg(2);
    else
      func = keyGet(getarg(2));
  }

  if (func && !Shell.defined(func)) {
    sp("Must be the name of function: ");
    sp(func);
    return 0;
  }

  Scout.scheduleSleep(getarg(1), func ? strdup(func) : NULL);

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

static numvar powerWakeupPin(void) {
  if (!checkArgs(1, 2, F("usage: power.wakeup.pin(\"pinName\", [enable])"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  bool enable = getarg(0) == 2 ? getarg(2) : 1;

  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (!SleepHandler::pinWakeupSupported(pin)) {
    speol(F("Wakeup not supported for this pin"));
    return 0;
  }

  if (Scout.isPinReserved(pin)) {
    speol(F("Cannot enable wakeup on a reserved pin"));
    return 0;
  }

  if (!Scout.isInputPin(pin)) {
    speol(F("Pin must be configured as input"));
    return 0;
  }

  SleepHandler::setPinWakeup(pin, enable);

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

static numvar ledBeccaPurple(void) {
  if (getarg(0) == 2) {
    Led.blinkBeccaPurple(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    Led.blinkBeccaPurple(getarg(1));
  } else {
    Led.beccapurple();
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
    int key = (isstringarg(i)) ? keyMap((char*)getstringarg(i), 1) : getarg(i);
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
          Scout.getPinMode(D2),
          Scout.getPinMode(D3),
          Scout.getPinMode(D4),
          Scout.getPinMode(D5),
          Scout.getPinMode(D6),
          Scout.getPinMode(D7),
          Scout.getPinMode(D8),
          Scout.pinStates[D2],
          Scout.pinStates[D3],
          Scout.pinStates[D4],
          Scout.pinStates[D5],
          Scout.pinStates[D6],
          Scout.pinStates[D7],
          Scout.pinStates[D8]);
  return Scout.handler.report(report);
}

static StringBuffer analogPinReportHQ(void) {
  StringBuffer report(100);
  report.appendSprintf("[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d,%d]]]",
          keyMap("analog", 0),
          keyMap("mode", 0),
          keyMap("state", 0),
          Scout.getPinMode(A0),
          Scout.getPinMode(A1),
          Scout.getPinMode(A2),
          Scout.getPinMode(A3),
          Scout.getPinMode(A4),
          Scout.getPinMode(A5),
          Scout.getPinMode(A6),
          Scout.getPinMode(A7),
          Scout.pinStates[A0],
          Scout.pinStates[A1],
          Scout.pinStates[A2],
          Scout.pinStates[A3],
          Scout.pinStates[A4],
          Scout.pinStates[A5],
          Scout.pinStates[A6],
          Scout.pinStates[A7]);
  return Scout.handler.report(report);
}

static numvar pinConstHigh(void) {
  return HIGH;
}

static numvar pinConstLow(void) {
  return LOW;
}

static numvar pinConstDisconnected(void) {
  return PinoccioScout::PINMODE_DISCONNECTED;
}

static numvar pinConstDisabled(void) {
  return PinoccioScout::PINMODE_DISABLED;
}

static numvar pinConstInput(void) {
  return PinoccioScout::PINMODE_INPUT;
}

static numvar pinConstOutput(void) {
  return PinoccioScout::PINMODE_OUTPUT;
}

static numvar pinConstInputPullup(void) {
  return PinoccioScout::PINMODE_INPUT_PULLUP;
}

static numvar pinConstPWM(void) {
  return PinoccioScout::PINMODE_PWM;
}

static numvar pinSetModeInternal(uint8_t pinarg, int8_t mode) {
  int8_t pin = getPinFromArg(pinarg);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (Scout.isPinReserved(pin)) {
    speol(F("Cannot change mode of reserved pin"));
    return 0;
  }

  if (mode == PinoccioScout::PINMODE_PWM && !Scout.isPWMPin(pin)) {
    speol(F("PWM mode not supported on this pin"));
    return 0;
  }

  if (!Scout.getNameForPinMode(mode)
      || mode == PinoccioScout::PINMODE_UNSET
      || mode == PinoccioScout::PINMODE_RESERVED) {
    speol(F("Invalid pin mode"));
    return 0;
  }

  if (!Scout.setMode(pin, mode)) {
    speol(F("Failed to change pin mode"));
    return 0;
  }

  return 1;
}

static numvar pinMakeInput(void) {
  if (!checkArgs(1, 2, F("usage: pin.makeinput(\"pinName\", inputType=INPUT_PULLUP)"))) {
    return 0;
  }

  int8_t mode = PinoccioScout::PINMODE_INPUT_PULLUP;
  if (getarg(0) == 2 && getarg(2) == PinoccioScout::PINMODE_INPUT) {
    mode = PinoccioScout::PINMODE_INPUT;
  }

  return pinSetModeInternal(1, mode);
}

static numvar pinMakeOutput(void) {
  if (!checkArgs(1, F("usage: pin.makeoutput(\"pinName\")"))) {
    return 0;
  }

  return pinSetModeInternal(1, PinoccioScout::PINMODE_OUTPUT);
}

static numvar pinMakePWM(void) {
  if (!checkArgs(1, F("usage: pin.makepwm(\"pinName\")"))) {
    return 0;
  }

  return pinSetModeInternal(1, PinoccioScout::PINMODE_PWM);
}

static numvar pinMakeDisconnected(void) {
  if (!checkArgs(1, F("usage: pin.makedisconnected(\"pinName\")"))) {
    return 0;
  }

  return pinSetModeInternal(1, PinoccioScout::PINMODE_PWM);
}

static numvar pinDisable(void) {
  if (!checkArgs(1, F("usage: pin.disable(\"pinName\")"))) {
    return 0;
  }

  return pinSetModeInternal(1, PinoccioScout::PINMODE_DISABLED);
}

static numvar pinSetMode(void) {
  if (!checkArgs(2, F("usage: pin.setmode(\"pinName\", pinMode)"))) {
    return 0;
  }

  return pinSetModeInternal(1, getarg(2));
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
  if (!checkArgs(2, F("usage: pin.write(\"pinName\", pinValue)"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  int16_t value = getarg(2);
  if (pin == -1) {
    speol(F("Invalid pin number"));
    return 0;
  }

  if (!Scout.isOutputPin(pin)) {
    speol(F("Pin must be set as an output before writing"));
    return 0;
  }
  if (Scout.getPinMode(pin) == PinoccioScout::PINMODE_PWM && (value < 0 || value > 255)) {
    speol(F("Invalid PWM value"));
    return 0;
  } 
  if (Scout.getPinMode(pin) != PinoccioScout::PINMODE_PWM && (value < 0 || value > 1)) {
    speol(F("Invalid pin value"));
    return 0;
  }

  Scout.pinWrite(pin, value);
  return 1;
}

static numvar pinStatus(void) {
  if (!checkArgs(0, F("usage: pin.status"))) {
    return 0;
  }

  // TODO: This should use sp/speol, but that doesn't return the number
  // of characters printed to use for alignment...
  Serial.println(F("Note: pin.status currently only works on Serial"));
  Serial.println(F("#   name    mode            value"));
  Serial.println(F("---------------------------------"));
  for (uint8_t pin = 0; pin < NUM_DIGITAL_PINS; ++pin) {
    printSpaces(4 - Serial.print(pin));
    printSpaces(8 - Serial.print(Scout.getNameForPin(pin)));
    int8_t mode = Scout.getPinMode(pin);
    printSpaces(16 - Serial.print(Scout.getNameForPinMode(mode) ?: F("unknown")));
    if (mode < 0)
      printSpaces(8 - Serial.print('-'));
    else
      printSpaces(8 - Serial.print(Scout.pinRead(pin)));

    const char *prefix = "";
    if (Scout.isPWMPin(pin)) {
      Serial.print(prefix);
      Serial.print("supports PWM");
      prefix = ", ";
    }
    if (SleepHandler::pinWakeupSupported(pin)) {
      Serial.print(prefix);
      Serial.print("supports wakeup");
      prefix = ", ";
    }
    if (SleepHandler::pinWakeupEnabled(pin)) {
      Serial.print(prefix);
      Serial.print("wakeup enabled");
      prefix = ", ";
    }

    Serial.println();
  }

  return 1;
}

static numvar pinNumber(void) {
  if (!checkArgs(1, F("usage: pin.number(\"pinName\")"))) {
    return 0;
  }

  return getPinFromArg(1);
}

static numvar pinSave(void) {
  if (!checkArgs(2, 3, F("usage: pin.save(\"pinName\", pinMode, [pinValue])"))) {
    return 0;
  }

  int8_t pin = getPinFromArg(1);
  int8_t mode = getarg(2);

  if (!pinSetModeInternal(1, mode))
    return 0;

  StringBuffer buf(128);

  buf += "function startup.";
  buf += Scout.getNameForPin(pin);
  buf += " { pin.setmode(\"";
  buf += Scout.getNameForPin(pin);
  buf += "\", ";
  buf += Scout.getNameForPinMode(mode);
  buf += ");";
  // if third arg is passed in, and mode is OUTPUT, then set pin value
  if (getarg(0) == 3 && mode == OUTPUT) {
    uint8_t value = getarg(3);
    Scout.pinWrite(pin, value);
    buf += " { pin.write(\"";
    buf += Scout.getNameForPin(pin);
    buf += "\", ";
    buf += value;
    buf += ");";
  }
  buf += " }";

  doCommand((char*)buf.c_str());
  return 1;
}

static numvar pinOthersDisconnected(void) {
  if (!checkArgs(0, 0, F("usage: pin.othersdisconnected()"))) {
    return 0;
  }
  Scout.makeUnsetDisconnected();
}

static numvar digitalPinReport(void) {
  speol(digitalPinReportHQ());
  return 1;
}

static numvar analogPinReport(void) {
  speol(analogPinReportHQ());
  return 1;
}

static int8_t getPinFromArg(uint8_t arg) {
  if (isstringarg(arg)) {
    return Scout.getPinFromName((const char*)getstringarg(arg));
  } else if (getarg(arg) >= 0 && getarg(arg) < NUM_DIGITAL_PINS) {
    return getarg(arg);
  } else {
    return -1;
  }
}

bool checkArgs(uint8_t exactly, const __FlashStringHelper *errorMsg) {
  if (getarg(0) != exactly) {
      speol(errorMsg);
      return false;
  }
  return true;
}

bool checkArgs(uint8_t min, uint8_t max, const __FlashStringHelper *errorMsg) {
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

  StringBuffer report(100);
  report.appendSprintf("[%d,[%d],[\"bye\"]]",keyMap("daisy",0),keyMap("dave",0));
  Scout.handler.report(report);

  if (ModuleHandler::loaded("wifi")) {
    WifiModule *wifi = (WifiModule*)ModuleHandler::load("wifi");
    if (!wifi->runDirectCommand(Serial, "AT&F")) {
       sp(F("Error: Wi-Fi direct command failed"));
       ret = false;
    }
    if (!wifi->runDirectCommand(Serial, "AT&W0")) {
       sp(F("Error: Wi-Fi direct command failed"));
       ret = false;
    }
  }

  if (ret == true) {
    Scout.meshResetSecurityKey();
    Scout.meshSetRadio(0, 0x0000);
    Scout.resetHQToken();
    Led.saveTorch(0, 255, 0);

    // so long, and thanks for all the fish!
    doCommand("rm *");
    doCommand("scout.boot");
  }
  return 1;
}

static numvar boot(void) {
  Scout.reboot();
}

static numvar otaBoot(void) {
  Scout.setOTAFlag();
  Scout.reboot();
}


/****************************\
 *    MODULES HANDLERS       *
\****************************/

static numvar moduleList(void) {
  ModuleHandler::list();
  return 1;
}

static numvar moduleLoad(void) {
  if (!checkArgs(1, F("usage: module.load(\"string\""))) {
    return 0;
  }
  if(ModuleHandler::load((char*)getstringarg(1))) return 1;
  return 0;
}

static numvar moduleLoaded(void) {
  ModuleHandler::loaded();
  return 1;
}

/****************************\
 *        HQ HANDLERS       *
\****************************/

static numvar hqBridge(void) {
  if(getarg(0) == 0)
  {
    Scout.handler.setBridged(true);
  }else{
    Scout.handler.bridge += (char*)getarg(1);
  }
  return 1;
}

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
  if (!checkArgs(2, 255, F("usage: hq.report(\"reportname\", \"value\")[,\"value\"...]"))) {
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
  if (Shell.defined(buf)) {
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
  if (Shell.defined(buf)) {
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

static void digitalPinEventHandler(uint8_t pin, int16_t value, int8_t mode) {
  uint32_t time = millis();
  char buf[16];
  
  digitalPinReportHQ();

  snprintf(buf, sizeof(buf), "on.d%d", pin);
  if (Shell.defined(buf)) {
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
    if (Shell.defined(buf)) {
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
  if (Shell.defined(buf)) {
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

  if (Shell.defined(func)) {
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

  if (Shell.defined(func)) {
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

  if (Shell.defined(func)) {
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
