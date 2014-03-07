#include "Shell.h"
#include "Scout.h"
#include "backpacks/Backpacks.h"
#include "bitlash.h"
#include "src/bitlash.h"
extern "C" {
#include "key/key.h"
}

static numvar pinoccioBanner(void);

static numvar getTemperature(void);
static numvar getRandomNumber(void);
static numvar uptimeReport(void);
static numvar allReport(void);
static numvar allVerbose(void);

static numvar isBatteryCharging(void);
static numvar getBatteryPercentage(void);
static numvar getBatteryVoltage(void);
static numvar enableBackpackVcc(void);
static numvar disableBackpackVcc(void);
static numvar goToSleep(void);
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
static numvar meshResetKey(void);
static numvar meshJoinGroup(void);
static numvar meshLeaveGroup(void);
static numvar meshIsInGroup(void);
static numvar meshPing(void);
static numvar meshPingGroup(void);
static numvar meshSend(void);
static numvar meshVerbose(void);
static numvar meshReport(void);
static numvar meshRouting(void);
static numvar meshAnnounce(void);
static numvar meshSignal(void);
static numvar meshLoss(void);

static numvar pinConstHigh(void);
static numvar pinConstLow(void);
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
static numvar daisyWipe(void);
static numvar boot(void);
static numvar otaBoot(void);

static numvar hqVerbose(void);

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

static numvar keyMap(void);
static numvar keyPrint(void);
static numvar keyNumber(void);
static numvar keySave(void);

static int getPinFromArg(int arg);

static char *scoutReportHQ(void);
static char *uptimeReportHQ(void);
static char *powerReportHQ(void);
static char *backpackReportHQ(void);
static char *digitalPinReportHQ(void);
static char *analogPinReportHQ(void);
static char *meshReportHQ(void);
static char *tempReportHQ(void);
static char *ledReportHQ(void);

static void pingScout(int address);
static void pingGroup(int address);
static void pingConfirm(NWK_DataReq_t *req);
static bool receiveMessage(NWK_DataInd_t *ind);
static void sendMessage(int address, char *data);
static void sendConfirm(NWK_DataReq_t *req);

static void digitalPinEventHandler(uint8_t pin, int8_t value, int8_t mode);
static void analogPinEventHandler(uint8_t pin, int16_t value, int8_t mode);
static void batteryPercentageEventHandler(uint8_t value);
static void batteryVoltageEventHandler(uint8_t value);
static void batteryChargingEventHandler(uint8_t value);
static void temperatureEventHandler(uint8_t value);
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
  addBitlashFunction("power.percent", (bitlash_function) getBatteryPercentage);
  addBitlashFunction("power.voltage", (bitlash_function) getBatteryVoltage);
  addBitlashFunction("power.enablevcc", (bitlash_function) enableBackpackVcc);
  addBitlashFunction("power.disablevcc", (bitlash_function) disableBackpackVcc);
  addBitlashFunction("power.sleep", (bitlash_function) goToSleep);
  addBitlashFunction("power.report", (bitlash_function) powerReport);

  addBitlashFunction("mesh.config", (bitlash_function) meshConfig);
  addBitlashFunction("mesh.setpower", (bitlash_function) meshSetPower);
  addBitlashFunction("mesh.setdatarate", (bitlash_function) meshSetDataRate);
  addBitlashFunction("mesh.key", (bitlash_function) meshSetKey);
  addBitlashFunction("mesh.resetkey", (bitlash_function) meshResetKey);
  addBitlashFunction("mesh.joingroup", (bitlash_function) meshJoinGroup);
  addBitlashFunction("mesh.leavegroup", (bitlash_function) meshLeaveGroup);
  addBitlashFunction("mesh.ingroup", (bitlash_function) meshIsInGroup);
  addBitlashFunction("mesh.ping", (bitlash_function) meshPing);
  addBitlashFunction("mesh.pinggroup", (bitlash_function) meshPingGroup);
  addBitlashFunction("mesh.send", (bitlash_function) meshSend);
  addBitlashFunction("mesh.verbose", (bitlash_function) meshVerbose);
  addBitlashFunction("mesh.report", (bitlash_function) meshReport);
  addBitlashFunction("mesh.routing", (bitlash_function) meshRouting);
  addBitlashFunction("mesh.announce", (bitlash_function) meshAnnounce);
  addBitlashFunction("mesh.signal", (bitlash_function) meshSignal);
  addBitlashFunction("mesh.loss", (bitlash_function) meshLoss);

  addBitlashFunction("temperature", (bitlash_function) getTemperature);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);
  addBitlashFunction("uptime", (bitlash_function) uptimeReport);
  addBitlashFunction("report", (bitlash_function) allReport);
  addBitlashFunction("verbose", (bitlash_function) allVerbose);

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

  addBitlashFunction("pin.high", (bitlash_function) pinConstHigh);
  addBitlashFunction("pin.low", (bitlash_function) pinConstLow);
  addBitlashFunction("pin.input", (bitlash_function) pinConstInput);
  addBitlashFunction("pin.output", (bitlash_function) pinConstOutput);
  addBitlashFunction("pin.inputpullup", (bitlash_function) pinConstInputPullup);
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
  addBitlashFunction("scout.sethqtoken", (bitlash_function) setHQToken);
  addBitlashFunction("scout.gethqtoken", (bitlash_function) getHQToken);
  addBitlashFunction("scout.delay", (bitlash_function) scoutDelay);
  addBitlashFunction("scout.daisy", (bitlash_function) daisyWipe);
  addBitlashFunction("scout.boot", (bitlash_function) boot);
  addBitlashFunction("scout.otaboot", (bitlash_function) otaBoot);

  addBitlashFunction("hq.settoken", (bitlash_function) setHQToken);
  addBitlashFunction("hq.gettoken", (bitlash_function) getHQToken);
  addBitlashFunction("hq.verbose", (bitlash_function) hqVerbose);

  addBitlashFunction("events.start", (bitlash_function) startStateChangeEvents);
  addBitlashFunction("events.stop", (bitlash_function) stopStateChangeEvents);
  addBitlashFunction("events.setcycle", (bitlash_function) setEventCycle);
  addBitlashFunction("events.verbose", (bitlash_function) setEventVerbose);

  addBitlashFunction("key", (bitlash_function) keyMap);
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
  }

  // set up event handlers
  Scout.digitalPinEventHandler = digitalPinEventHandler;
  Scout.analogPinEventHandler = analogPinEventHandler;
  Scout.batteryPercentageEventHandler = batteryPercentageEventHandler;
  Scout.batteryVoltageEventHandler = batteryVoltageEventHandler;
  Scout.batteryChargingEventHandler = batteryChargingEventHandler;
  Scout.temperatureEventHandler = temperatureEventHandler;
  RgbLed.ledEventHandler = ledEventHandler;

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
  speol("Hello from Pinoccio!");
  speol(" (Shell based on Bitlash v2.0 (c) 2014 Bill Roy)");
  sp(" ");
  sp(func_free());
  speol(" bytes free");
  sp(" Build ");
  sp(PINOCCIO_BUILD);
  speol();

  if (Scout.isLeadScout()) {
    speol(" Lead Scout ready");
  } else {
    speol(" Field Scout ready");
  }
  return 1;
}

static numvar allReport(void) {
  sp("running all reports");
  speol();
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
  char boot[32];
  uint8_t i;

  isShellEnabled = true;
  initBitlash(115200);

  for (i='a'; i<'z'; i++) {
    sprintf(boot, "startup.%c", i);
    if (findscript(boot)) {
      doCommand(boot);
    }
  }

  for (i=2; i<9; i++) {
    sprintf(boot, "startup.d%d", i);
    if (findscript(boot)) {
      doCommand(boot);
    }
  }

  for (i=0; i<8; i++) {
    sprintf(boot, "startup.a%d", i);
    if (findscript(boot)) {
      doCommand(boot);
    }
  }
}

void PinoccioShell::disableShell() {
  isShellEnabled = false;
}

static NWK_DataReq_t pingDataReq;
static NWK_DataReq_t sendDataReq;
static bool sendDataReqBusy;
static int tempHigh = 0;
static int tempLow = 0;


/****************************\
*      BUILT-IN HANDLERS    *
\****************************/

static char *tempReportHQ(void) {
  static char report[100];
  int temp = Scout.getTemperature();
  if(temp > tempHigh) tempHigh = temp;
  if(!tempLow || temp < tempLow) tempLow = temp;
  sprintf(report,"[%d,[%d,%d,%d],[%d,%d,%d]]",
          keyMap("temp", 0),
          keyMap("current", 0),
          keyMap("high", 0),
          keyMap("low", 0),
          temp,
          tempHigh,
          tempLow);
  return Scout.handler.report(report);
}

static numvar getTemperature(void) {
  tempReportHQ();
  int i = Scout.getTemperature();
  speol(i);
  return i;
}

static numvar getRandomNumber(void) {
  int i = random();
  speol(i);
  return i;
}

extern int __bss_end;
static char *uptimeReportHQ(void) {
  static char report[100];
  int freeMem;

  char reset[20];
  char c;

  const char *resetString = Scout.getLastResetCause();
  reset[0] = 0;
  while((c = pgm_read_byte(resetString++))) {
    sprintf(reset + strlen(reset), "%c", c);
  }

  // free memory based on http://forum.pololu.com/viewtopic.php?f=10&t=989&view=unread#p4218
  sprintf(report,"[%d,[%d,%d,%d,%d],[%ld,%d,%d,\"",keyMap("uptime",0),
          keyMap("millis", 0),
          keyMap("free", 0),
          keyMap("random", 0),
          keyMap("reset", 0),
          (unsigned long)millis(),
          ((int)&freeMem) - ((int)&__bss_end),
          (int)random());

  sprintf(report + strlen(report),"%s\"]]", (char*)reset);
  return Scout.handler.report(report);
}

static numvar uptimeReport(void) {
  uptimeReportHQ();
  sp(millis());
  speol();
  return true;
}


/****************************\
*        KEY HANDLERS        *
\****************************/

static numvar keyMap(void) {
  static char num[8];
  if (isstringarg(1)) {
    return keyMap((char*)getstringarg(1), 0);
  }
  snprintf(num, 8, "%lu", getarg(1));
  return keyMap(num, 0);
}

static numvar keyPrint(void) {
  const char *key = keyGet(getarg(1));
  if (!key) {
    return 0;
  }
  speol(key);
  return 1;
}

static numvar keyNumber(void) {
  const char *key = keyGet(getarg(1));
  if (!key) {
    return 0;
  }
  return atoi(key);
}

static numvar keySave(void) {
  char cmd[42], *var;
  if (getarg(0) != 2 || !isstringarg(1)) {
    return 0;
  }
  var = (char*)getstringarg(1);
  sprintf(cmd, "function boot.%s {%s=key(\"%s\");}", var, var, keyGet(getarg(2)));
  doCommand(cmd);
  return 1;
}


/****************************\
*      POWER HANDLERS       *
\****************************/

static numvar isBatteryCharging(void) {
  int i = Scout.isBatteryCharging();
  speol(i);
  return i;
}

static numvar getBatteryPercentage(void) {
  int i = Scout.getBatteryPercentage();
  speol(i);
  return i;
}

static numvar getBatteryVoltage(void) {
  int i = Scout.getBatteryVoltage();
  speol(i);
  return i;
}

static numvar enableBackpackVcc(void) {
  Scout.enableBackpackVcc();
  return true;
}

static numvar disableBackpackVcc(void) {
  Scout.disableBackpackVcc();
  return true;
}

static numvar goToSleep(void) {
  // TODO: not implemented yet
  //Pinoccio.goToSleep(getarg(1));
  return 1;
}

static char *powerReportHQ(void) {
  static char report[100];
  sprintf(report,"[%d,[%d,%d,%d,%d],[%d,%d,%s,%s]]",
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
  return true;
}

/****************************\
*      RGB LED HANDLERS     *
\****************************/

static char *ledReportHQ(void) {
  static char report[100];
  sprintf(report,"[%d,[%d,%d],[[%d,%d,%d],[%d,%d,%d]]]",
          keyMap("led", 0),
          keyMap("led", 0),
          keyMap("torch", 0),
          RgbLed.getRedValue(),
          RgbLed.getGreenValue(),
          RgbLed.getBlueValue(),
          RgbLed.getRedTorchValue(),
          RgbLed.getGreenTorchValue(),
          RgbLed.getBlueTorchValue());
  return Scout.handler.report(report);
}

static numvar ledBlink(void) {
  if (getarg(0) == 5) {
    RgbLed.blinkColor(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5));
  } else if (getarg(0) == 4) {
    RgbLed.blinkColor(getarg(1), getarg(2), getarg(3), getarg(4));
  } else {
    RgbLed.blinkColor(getarg(1), getarg(2), getarg(3));
  }
  return 1;
}

static numvar ledOff(void) {
  RgbLed.turnOff();
  return 1;
}

static numvar ledRed(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkRed(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkRed(getarg(1));
  } else {
    RgbLed.red();
  }
  return 1;
}

static numvar ledGreen(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkGreen(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkGreen(getarg(1));
  } else {
    RgbLed.green();
  }
  return 1;
}

static numvar ledBlue(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkBlue(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkBlue(getarg(1));
  } else {
    RgbLed.blue();
  }
  return 1;
}

static numvar ledCyan(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkCyan(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkCyan(getarg(1));
  } else {
    RgbLed.cyan();
  }
  return 1;
}

static numvar ledPurple(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkPurple(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkPurple(getarg(1));
  } else {
    RgbLed.purple();
  }
  return 1;
}

static numvar ledMagenta(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkMagenta(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkMagenta(getarg(1));
  } else {
    RgbLed.magenta();
  }
  return 1;
}

static numvar ledYellow(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkYellow(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkYellow(getarg(1));
  } else {
    RgbLed.yellow();
  }
  return 1;
}

static numvar ledOrange(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkOrange(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkOrange(getarg(1));
  } else {
    RgbLed.orange();
  }
  return 1;
}

static numvar ledWhite(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkWhite(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkWhite(getarg(1));
  } else {
    RgbLed.white();
  }
  return 1;
}

static numvar ledGetHex(void) {
  char hex[8];
  sprintf(hex,"%02x%02x%02x", RgbLed.getRedValue(), RgbLed.getGreenValue(), RgbLed.getBlueValue());
  return keyMap(hex, millis());
}

static numvar ledSetHex(void) {
  if (getarg(1)) {
    const char *str;
    if (isstringarg(1)) {
      str = (const char *)getarg(1);
    } else {
      str = keyGet(getarg(1));
    }

    uint8_t out[3];
    PinoccioShell::parseHex(str, 6, out);
    RgbLed.setColor(out[0], out[1], out[2]);

    return true;
  } else {
    return false;
  }
  return 1;
}

static numvar ledSetRgb(void) {
  RgbLed.setColor(getarg(1), getarg(2), getarg(3));
  return 1;
}

static numvar ledIsOff(void) {
  return RgbLed.isOff();
}

static numvar ledSaveTorch(void) {
  RgbLed.saveTorch(getarg(1), getarg(2), getarg(3));
  return 1;
}

static numvar ledTorch(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkTorch(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkTorch(getarg(1));
  } else {
    RgbLed.setTorch();
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
  uint16_t panId = 0x4567;
  uint8_t channel = 20;
  if (getarg(0) >= 2) {
    panId = getarg(2);
  }
  if (getarg(0) >= 3) {
    channel = getarg(3);
  }
  Scout.meshSetRadio(getarg(1), panId, channel);
  return 1;
}

static numvar meshSetPower(void) {
  Scout.meshSetPower(getarg(1));
  return 1;
}

static numvar meshSetDataRate(void) {
  Scout.meshSetDataRate(getarg(1));
  return 1;
}

static numvar meshSetKey(void) {
  Scout.meshSetSecurityKey((const uint8_t *)getstringarg(1));
  return 1;
}

static numvar meshResetKey(void) {
  Scout.meshResetSecurityKey();
  return 1;
}

static numvar meshJoinGroup(void) {
  Scout.meshJoinGroup(getarg(1));
  return 1;
}

static numvar meshLeaveGroup(void) {
  Scout.meshLeaveGroup(getarg(1));
  return 1;
}

static numvar meshIsInGroup(void) {
  return Scout.meshIsInGroup(getarg(1));
}

static numvar meshPing(void) {
  pingScout(getarg(1));
  return 1;
}

static numvar meshPingGroup(void) {
  pingGroup(getarg(1));
  return 1;
}

char *arg2array(int ver, char *msg) {
  int i;
  int args = getarg(0);
  if (args > 8) {
    args = 8;
  }
  sprintf(msg,"[%d,",ver);
  for (i=2; i<=args; i++) {
    int key = (isstringarg(i)) ? keyMap((char*)getstringarg(i), 0) : getarg(i);
    sprintf(msg + strlen(msg), "\"%s\",", keyGet(key));
  }
  sprintf(msg + (strlen(msg)-1), "]");
  return msg;
}

static numvar meshSend(void) {
  char msg[100];
  if (!getarg(0)) {
    return false;
  }

  sendMessage(getarg(1), arg2array(1, msg));
  return true;
}

static numvar meshAnnounce(void) {
  char msg[100];
  if (!getarg(0)) {
    return false;
  }
  Scout.handler.announce(getarg(1), arg2array(1, msg));
  return true;
}

static numvar meshSignal(void) {
  return lastMeshRssi;
}

static numvar meshLoss(void) {
  return lastMeshLqi;
}

static numvar meshVerbose(void) {
  isMeshVerbose = getarg(1);
  return 1;
}

static char *meshReportHQ(void) {
  static char report[100], c;
  int count = 0;
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i<NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) continue;
    count++;
  }
  sprintf(report, "[%d,[%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,\"",
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
  while (c = pgm_read_byte(kbString++)) {
    sprintf(report + strlen(report),"%c", c);
  }

  sprintf(report + strlen(report), "\",\"");

  const char *dbString = Scout.getTxPowerDb();
  while (c = pgm_read_byte(dbString++)) {
    sprintf(report + strlen(report), "%c", c);
  }
  sprintf(report + strlen(report), "\"]]");
  return Scout.handler.report(report);
}

static numvar meshReport(void) {
  speol(meshReportHQ());
  return 1;
}

static numvar meshRouting(void) {
  sp(" - Routing: ");
  sp("|    Fixed    |  Multicast  |    Score    |    DstAdd   | NextHopAddr |    Rank     |     LQI    |");
  speol();
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i < NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) {
      continue;
    }
    sp("|      ");
    sp(table[i].fixed);
    sp("      |      ");
    sp(table[i].multicast);
    sp("      |      ");
    sp(table[i].score);
    sp("      |     ");
    sp(table[i].dstAddr);
    sp("     |     ");
    sp(table[i].nextHopAddr);
    sp("     |     ");
    sp(table[i].rank);
    sp("     |     ");
    sp(table[i].lqi);
    sp("    |");
    speol();
  }
  return 1;
}

/****************************\
*        I/O HANDLERS       *
\****************************/

static char *digitalPinReportHQ(void) {
  static char report[80];
  sprintf(report,"[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d]]]",
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

static char *analogPinReportHQ(void) {
  static char report[80];
  sprintf(report,"[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d,%d]]]",
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
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol("Invalid pin number");
    return 0;
  }

  bool pullup = true;
  if (getarg(0) == 2 && getarg(2) == 0) {
    pullup = false;
  }

  if (!Scout.makeInput(pin, pullup)) {
    speol("Cannot change mode of reserved pin");
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
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol("Invalid pin number");
    return 0;
  }

  if (!Scout.makeOutput(pin)) {
    speol("Cannot change mode of reserved pin");
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
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol("Invalid pin number");
    return 0;
  }

  if (!Scout.makeDisabled(pin)) {
    speol("Cannot change mode of reserved pin");
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
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol("Invalid pin number");
    return 0;
  }

  if (!Scout.setMode(pin, getarg(2))) {
    speol("Cannot change mode of reserved pin");
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
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol("Invalid pin number");
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    return digitalRead(getarg(1));
  } else if (Scout.isAnalogPin(pin)) {
    return analogRead(getarg(1));
  } else {
    return 0;
  }
}

static numvar pinWrite(void) {
  // TODO: handle PWM pins
  int8_t pin = getPinFromArg(1);
  if (pin == -1) {
    speol("Invalid pin number");
    return 0;
  }

  if (Scout.isDigitalPin(pin)) {
    Scout.makeOutput(pin);
    digitalWrite(pin, getarg(2));
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    Scout.makeOutput(pin);
    digitalWrite(pin, getarg(2));
    analogPinReportHQ();
  }
  return true;
}

static numvar pinSave(void) {
  char buf[64];
  const char *str = (const char*)getstringarg(1);
  sprintf(buf, "function startup.%s { pin.setmode(\"%s\",%d) }", str, str, getarg(2));
  doCommand(buf);
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

/****************************\
*     BACKPACK HANDLERS     *
\****************************/

static char *backpackReportHQ(void) {
  static char report[100];
  int comma = 0;
  sprintf(report, "[%d,[%d],[[", keyMap("backpacks", 0), keyMap("list", 0));

  for (uint8_t i=0; i<Backpacks::num_backpacks; ++i) {
    BackpackInfo &info = Backpacks::info[i];
    for (uint8_t j=0; j<sizeof(info.id); ++j) {
      // TODO this isn't correct, dunno what to do here
      sprintf(report+strlen(report),"%s%d",comma++?",":"",info.id.raw_bytes[j]);
    }
  }
  sprintf(report + strlen(report), "]]]");
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
    Serial.println("No backpacks found");
  } else {
    for (uint8_t i=0; i<Backpacks::num_backpacks; ++i) {
      BackpackInfo &info = Backpacks::info[i];
      printHexBuffer(Serial, &i, 1);
      sp(": ");

      Pbbe::Header *h = info.getHeader();
      if (!h) {
        sp(F("Error parsing name"));
      } else {
        sp(h->backpack_name);
      }

      sp(" (");
      printHexBuffer(Serial, info.id.raw_bytes, sizeof(info.id));
      speol(")");
    }
  }
  return 0;
}

static numvar backpackEeprom(void) {
  numvar addr = getarg(1);
  if (addr < 0 || addr >= Backpacks::num_backpacks) {
    Serial.println("Invalid backpack number");
    return 0;
  }

  // Get EEPROM contents
  Pbbe::Eeprom *eep = Backpacks::info[addr].getEeprom();
  if (!eep) {
    Serial.println("Failed to fetch EEPROM");
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
    Serial.println("Invalid backpack number");
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
    Serial.println(F("Failed to update EEPROM"));
    return 0;
  }
  // Update the eep pointer, it might have been realloced
  info.eep = eep;

  if (!Pbbe::writeEeprom(Backpacks::pbbp, addr, info.eep)) {
    Serial.println(F("Failed to write EEPROM"));
    return 0;
  }
  return 1;
}


static numvar backpackDetail(void) {
  numvar addr = getarg(1);
  if (addr < 0 || addr >= Backpacks::num_backpacks) {
    Serial.println("Invalid backpack number");
    return 0;
  }
  Pbbe::Header *h = Backpacks::info[addr].getHeader();
  Pbbe::UniqueId &id = Backpacks::info[addr].id;

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
    Serial.println("Invalid backpack number");
    return 0;
  }

  Pbbe::DescriptorList *list = Backpacks::info[addr].getAllDescriptors();
  if (!list) {
    Serial.println("Failed to fetch or parse resource descriptors");
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
        Serial.print(": spi, ss = ");
        Serial.print(d.ss_pin.name());
        Serial.print(", max speed = ");
        if (d.speed.raw()) {
          Serial.print((float)d.speed, 2);
          Serial.print("Mhz");
        } else {
          Serial.print("unknown");
        }
        Serial.println();
        break;
      }
      case Pbbe::DT_UART: {
        Pbbe::UartDescriptor& d = static_cast<Pbbe::UartDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(": uart, tx = ");
        Serial.print(d.tx_pin.name());
        Serial.print(", rx = ");
        Serial.print(d.rx_pin.name());
        Serial.print(", speed = ");
        if (d.speed) {
          Serial.print(d.speed);
          Serial.print("bps");
        } else {
          Serial.print("unknown");
        }
        Serial.println();
        break;
      }
      case Pbbe::DT_IOPIN: {
        Pbbe::IoPinDescriptor& d = static_cast<Pbbe::IoPinDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(": gpio, pin = ");
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
        Serial.print("power: pin = ");
        Serial.print(d.power_pin.name());
        Serial.print(", minimum = ");
        if (d.minimum.raw()) {
          Serial.print((float)d.minimum, 2);
          Serial.print("uA");
        } else {
          Serial.print("unknown");
        }
        Serial.print(", typical = ");
        if (d.typical.raw()) {
          Serial.print((float)d.typical, 2);
          Serial.print("uA");
        } else {
          Serial.print("unknown");
        }
        Serial.print(", maximum = ");
        if (d.maximum.raw()) {
          Serial.print((float)d.maximum, 2);
          Serial.print("uA");
        } else {
          Serial.print("unknown");
        }
        Serial.println();
        break;
      }
      case Pbbe::DT_I2C_SLAVE: {
        Pbbe::I2cSlaveDescriptor& d = static_cast<Pbbe::I2cSlaveDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(": i2c, address = ");
        Serial.print(d.addr);
        Serial.print(", max speed = ");
        Serial.print(d.speed);
        Serial.print("kbps");
        Serial.println();
        break;
      }
      case Pbbe::DT_DATA: {
        Pbbe::DataDescriptor& d = static_cast<Pbbe::DataDescriptor&>(*info.parsed);
        Serial.print(d.name);
        Serial.print(": data, length = ");
        Serial.print(d.length);
        Serial.print(", content = ");
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

static char *scoutReportHQ(void) {
  static char report[100];
  sprintf(report,"[%d,[%d,%d,%d,%d,%d,%d],[%s,%d,%d,%d,%ld,%ld]]",
          keyMap("scout", 0),
          keyMap("lead", 0),
          keyMap("version", 0),
          keyMap("hardware", 0),
          keyMap("family", 0),
          keyMap("serial", 0),
          keyMap("build", 0),
          Scout.isLeadScout() ? "true" : "false",
          (int)Scout.getEEPROMVersion(),
          (int)Scout.getHwVersion(),
          Scout.getHwFamily(),
          Scout.getHwSerial(),
          PINOCCIO_BUILD);
  return Scout.handler.report(report);
}

static numvar scoutReport(void) {
  speol(scoutReportHQ());
  return 1;
}

static numvar isScoutLeadScout(void) {
  speol(Scout.isLeadScout() ? 1 : 0);
  return Scout.isLeadScout();
}

static numvar setHQToken(void) {
  Pinoccio.setHQToken((const char *)getstringarg(1));
  return 1;
}

static numvar getHQToken(void) {
  char token[33];
  Pinoccio.getHQToken((char *)token);
  token[32] = 0;
  speol(token);
  return 1;
}

static numvar scoutDelay(void) {
  Scout.delay(getarg(1));
  return 1;
}

static numvar daisyWipe(void) {
  bool ret = true;

  if (Scout.factoryReset() == false) {
    speol("Factory reset requested. Send command again to confirm.");
    return false;
  } else {
    speol("Ok, terminating. Goodbye Dave.");
  }

  char report[32];
  sprintf(report,"[%d,[%d],[\"bye\"]]",keyMap("daisy",0),keyMap("dave",0));
  Scout.handler.report(report);

  if (Scout.isLeadScout()) {
    if (!Scout.wifi.runDirectCommand(Serial, "AT&F")) {
       sp("Error: Wi-Fi direct command failed");
       ret = false;
    }
    if (!Scout.wifi.runDirectCommand(Serial, "AT&W0")) {
       sp("Error: Wi-Fi direct command failed");
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

static char *wifiReportHQ(void) {
  static char report[100];
  // TODO real wifi status/version
  sprintf(report,"[%d,[%d,%d,%d],[%d,%s,%s]]",
          keyMap("wifi", 0),
          keyMap("version", 0),
          keyMap("connected", 0),
          keyMap("hq", 0),
          0,
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
    Serial.print("Wi-Fi Versions: ");
    Scout.wifi.printFirmwareVersions(Serial);
    Scout.wifi.printCurrentNetworkStatus(Serial);
  }
  return 1;
}

static numvar wifiList(void) {
  if (!Scout.wifi.printAPs(Serial)) {
    Serial.println("Error: Scan failed");
    return 0;
  }
  return 1;
}

static numvar wifiConfig(void) {
  if (!Scout.wifi.wifiConfig((const char *)getstringarg(1), (const char *)getstringarg(2))) {
    Serial.println("Error: saving Scout.wifi.configuration data failed");
  }
  return 1;
}

static numvar wifiDhcp(void) {
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!Scout.wifi.wifiDhcp(host)) {
    Serial.println("Error: saving Scout.wifi.configuration data failed");
  }
  return 1;
}

static numvar wifiStatic(void) {
  IPAddress ip, nm, gw, dns;

  if (!GSCore::parseIpAddress(&ip, (const char *)getstringarg(1))) {
    Serial.println("Error: Invalid IP address");
    return 0;
  }

  if (!GSCore::parseIpAddress(&nm, (const char *)getstringarg(2))) {
    Serial.println("Error: Invalid netmask");
    return 0;
  }

  if (!GSCore::parseIpAddress(&gw, (const char *)getstringarg(3))) {
    Serial.println("Error: Invalid gateway");
    return 0;
  }

  if (!GSCore::parseIpAddress(&dns, (const char *)getstringarg(3))) {
    Serial.println("Error: Invalid dns server");
    return 0;
  }

  if (!Scout.wifi.wifiStatic(ip, nm, gw, dns)) {
    Serial.println("Error: saving Scout.wifi.configuration data failed");
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
  if (!Scout.wifi.runDirectCommand(Serial, (const char *)getstringarg(1))) {
     Serial.println("Error: Wi-Fi direct command failed");
  }
  return 1;
}

static numvar wifiPing(void) {
  if (!Scout.wifi.ping(Serial, (const char *)getstringarg(1))) {
     Serial.println("Error: Wi-Fi ping command failed");
  }
  return 1;
}

static numvar wifiDNSLookup(void) {
  if (!Scout.wifi.dnsLookup(Serial, (const char *)getstringarg(1))) {
     Serial.println("Error: Wi-Fi DNS lookup command failed");
  }
  return 1;
}

static numvar wifiGetTime(void) {
  if (!Scout.wifi.printTime(Serial)) {
     Serial.println("Error: Wi-Fi NTP time lookup command failed");
  }
  return 1;
}

static numvar wifiSleep(void) {
  if (!Scout.wifi.goToSleep()) {
     Serial.println("Error: Wi-Fi sleep command failed");
  }
  return 1;
}

static numvar wifiWakeup(void) {
  if (!Scout.wifi.wakeUp()) {
     Serial.println("Error: Wi-Fi wakeup command failed");
  }
  return 1;
}

static numvar wifiVerbose(void) {
  // TODO
  return 1;
}


/****************************\
 *     HELPER FUNCTIONS     *
\****************************/

static void pingScout(int address) {
  pingDataReq.dstAddr = address;
  const char *ping = "ping";

  pingDataReq.dstEndpoint = 1;
  pingDataReq.srcEndpoint = 1;
  pingDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  pingDataReq.data = (byte *)ping;
  pingDataReq.size = strlen(ping);
  pingDataReq.confirm = pingConfirm;
  NWK_DataReq(&pingDataReq);

  Serial.print("PING ");
  Serial.println(address);
}

static void pingGroup(int address) {
  pingDataReq.dstAddr = address;
  const char *ping = "ping";

  pingDataReq.dstEndpoint = 1;
  pingDataReq.srcEndpoint = 1;
  pingDataReq.options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  pingDataReq.data = (byte *)ping;
  pingDataReq.size = strlen(ping);
  pingDataReq.confirm = pingConfirm;
  NWK_DataReq(&pingDataReq);

  Serial.print("PING ");
  Serial.println(address, HEX);
}

static void pingConfirm(NWK_DataReq_t *req) {
  Serial.print("dstAddr: ");
  Serial.println(req->dstAddr, HEX);
  Serial.print("dstEndpoint: ");
  Serial.println(req->dstEndpoint);
  Serial.print("srcEndpoint: ");
  Serial.println(req->srcEndpoint);
  Serial.print("options: ");
  Serial.println(req->options, BIN);
  Serial.print("size: ");
  Serial.println(req->size);
  Serial.print("status: ");
  Serial.println(req->status, HEX);

  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.print("1 byte from ");
    Serial.print(req->dstAddr);
    Serial.print(" RSSI=-");
    Serial.println(req->control);
  } else {
    Serial.print("Error: ");
    switch (req->status) {
      case NWK_OUT_OF_MEMORY_STATUS:
        Serial.print("Out of memory: ");
        break;
      case NWK_NO_ACK_STATUS:
      case NWK_PHY_NO_ACK_STATUS:
        Serial.print("No acknowledgement received: ");
        break;
      case NWK_NO_ROUTE_STATUS:
        Serial.print("No route to destination: ");
        break;
      case NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS:
        Serial.print("Physical channel access failure: ");
        break;
      default:
        Serial.print("unknown failure: ");
    }
    Serial.print("(");
    Serial.print(req->status, HEX);
    Serial.println(")");
  }
}

static bool receiveMessage(NWK_DataInd_t *ind) {
  char buf[64];
  char *data = (char*)ind->data;
  int keys[10];

  if (isMeshVerbose) {
    Serial.print("Received message of ");
    Serial.print(data);
    Serial.print(" from ");
    Serial.print(ind->srcAddr, DEC);
    Serial.print(" lqi ");
    Serial.print(ind->lqi, DEC);
    Serial.print(" rssi ");
    Serial.print(abs(ind->rssi), DEC);
    Serial.println("");
  }
  lastMeshRssi = abs(ind->rssi);
  lastMeshLqi = ind->lqi;
  NWK_SetAckControl(abs(ind->rssi));

  if (strlen(data) <3 || data[0] != '[') {
    return false;
  }

  // parse the array payload into keys, [1, "foo", "bar"]
  keyLoad(data, keys, millis());

  sprintf(buf,"event.message");
  if (findscript(buf)) {
    sprintf(buf, "event.message(%d", ind->srcAddr);
    for (int i=2; i<=keys[0]; i++) {
      sprintf(buf + strlen(buf), ",%d", keys[i]);
    }
    sprintf(buf + strlen(buf), ")");
    doCommand(buf);
  }
  return true;
}

static void sendMessage(int address, char *data) {
  if (sendDataReqBusy) {
    return;
  }

  sendDataReq.dstAddr = address;
  sendDataReq.dstEndpoint = 1;
  sendDataReq.srcEndpoint = 1;
  sendDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  sendDataReq.data = (uint8_t*)strdup(data);
  sendDataReq.size = strlen(data)+1;
  sendDataReq.confirm = sendConfirm;
  NWK_DataReq(&sendDataReq);

  sendDataReqBusy = true;

  if (isMeshVerbose) {
    sp("Sent message to Scout ");
    sp(address);
    sp(": ");
    speol(data);
  }
}

static void sendConfirm(NWK_DataReq_t *req) {
   sendDataReqBusy = false;
   free(req->data);

   if (isMeshVerbose) {
    if (req->status == NWK_SUCCESS_STATUS) {
      Serial.print("-  Message successfully sent to Scout ");
      Serial.print(req->dstAddr);
      if (req->control) {
        Serial.print(" (Confirmed with control byte: ");
        Serial.print(req->control);
        Serial.print(")");
      }
      Serial.println();
    } else {
      Serial.print("Error: ");
      switch (req->status) {
        case NWK_OUT_OF_MEMORY_STATUS:
          Serial.print("Out of memory: ");
          break;
        case NWK_NO_ACK_STATUS:
        case NWK_PHY_NO_ACK_STATUS:
          Serial.print("No acknowledgement received: ");
          break;
        case NWK_NO_ROUTE_STATUS:
          Serial.print("No route to destination: ");
          break;
        case NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS:
          Serial.print("Physical channel access failure: ");
          break;
        default:
          Serial.print("unknown failure: ");
      }
      Serial.print("(");
      Serial.print(req->status, HEX);
      Serial.println(")");
    }
  }
  lastMeshRssi = req->control;

  // run the Bitlash callback ack function
  char buf[32];
  sprintf(buf,"event.ack");
  if (findscript(buf)) {
    sprintf(buf, "event.ack(%d, %d)", req->dstAddr, (req->status == NWK_SUCCESS_STATUS) ? req->control : 0);
    doCommand(buf);
  }
}


/****************************\
 *      EVENT HANDLERS      *
\****************************/

static void digitalPinEventHandler(uint8_t pin, int8_t value, int8_t mode) {
  uint32_t time = millis();
  char buf[32];

  digitalPinReportHQ();
  if (findscript("event.digital")) {
    String callback = "event.digital(" + String(pin) + "," + String(value) + "," + String(mode) + ")";
    callback.toCharArray(buf, callback.length() + 1);
    doCommand(buf);
  }

  sprintf(buf, "event.digital%d", pin);
  if (findscript(buf)) {
    sprintf(buf, "event.digital%d(%d, %d)", pin, value, mode);
    doCommand(buf);
  }

  // simplified button trigger
  if (value == 0 && (mode == INPUT_PULLUP || mode == INPUT)) {
    sprintf(buf, "event.button%d", pin);
    if (findscript(buf)) {
      doCommand(buf);
    }
  }

  if (Scout.eventVerboseOutput) {
    Serial.print("Digital pin event handler took ");
    Serial.print(millis() - time);
    Serial.println("ms");
    Serial.println();
  }
}

static void analogPinEventHandler(uint8_t pin, int16_t value, int8_t mode) {
  uint32_t time = millis();
  char buf[32];

  analogPinReportHQ();
  if (findscript("event.analog")) {
    String callback = "event.analog(" + String(pin) + "," + String(value) + "," + String(mode) + ")";
    char buf[32];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }

  sprintf(buf,"event.analog%d", pin);
  if (findscript(buf)) {
    sprintf(buf, "event.analog%d(%d, %d)", pin, value, mode);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print("Analog pin event handler took ");
    Serial.print(millis() - time);
    Serial.println("ms");
    Serial.println();
  }
}

static void batteryPercentageEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.percent")) {
    String callback = "event.percent(" + String(value) + ")";
    char buf[32];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void batteryVoltageEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.voltage")) {
    String callback = "event.voltage(" + String(value) + ")";
    char buf[32];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void batteryChargingEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.charging")) {
    String callback = "event.charging(" + String(value) + ")";
    char buf[32];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void temperatureEventHandler(uint8_t value) {
  tempReportHQ();
  if (findscript("event.temperature")) {
    String callback = "event.temperature(" + String(value) + ")";
    char buf[32];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void ledEventHandler(uint8_t redValue, uint8_t greenValue, uint8_t blueValue) {
  ledReportHQ();
}

void bitlashFilter(byte b) {
  Serial.write(b);
  return;
}

void bitlashBuffer(byte b) {
  int len;

  Serial.write(b);
  if (b == '\r') {
    return;
  }

  len = strlen(Shell.bitlashOutput);
  Shell.bitlashOutput = (char*)realloc(Shell.bitlashOutput, len + 3); // up to 2 bytes w/ escaping, and the null term

  // escape newlines, quotes, and slashes
  if (b == '\n') {
    Shell.bitlashOutput[len++] = '\\';
    b = 'n';
  }

  if (b == '"') {
    Shell.bitlashOutput[len++] = '\\';
    b = '"';
  }

  if (b == '\\') {
    Shell.bitlashOutput[len++] = '\\';
    b = '\\';
  }
  Shell.bitlashOutput[len] = b;
  Shell.bitlashOutput[len+1] = 0;
}

int prepareBitlashBuffer() {
  if (Shell.bitlashOutput != NULL) {
    free(Shell.bitlashOutput);
    Shell.bitlashOutput = NULL;
  }

  Shell.bitlashOutput = (char*)malloc(1);
  Shell.bitlashOutput[0] = 0;
}
