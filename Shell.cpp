#include "Shell.h"
#include "Scout.h"
#include "bitlash.h"
#include "src/bitlash.h"
extern "C" {
#include "utility/key.h"
}

PinoccioShell Shell;

PinoccioShell::PinoccioShell() {
  isShellEnabled = true;
}

PinoccioShell::~PinoccioShell() { }

void PinoccioShell::setup() {
  key_init();
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
  addBitlashFunction("led.savetorch", (bitlash_function) ledSaveTorch);
  addBitlashFunction("led.report", (bitlash_function) ledReport);

  addBitlashFunction("pin.on", (bitlash_function) pinOn);
  addBitlashFunction("pin.off", (bitlash_function) pinOff);
  addBitlashFunction("pin.makeinput", (bitlash_function) pinMakeInput);
  addBitlashFunction("pin.makeinputup", (bitlash_function) pinMakeInputPullup);
  addBitlashFunction("pin.makeoutput", (bitlash_function) pinMakeOutput);
  addBitlashFunction("pin.setmode", (bitlash_function) pinSetMode);
  addBitlashFunction("pin.read", (bitlash_function) pinRead);
  addBitlashFunction("pin.write", (bitlash_function) pinWrite);
  addBitlashFunction("pin.report.digital", (bitlash_function) digitalPinReport);
  addBitlashFunction("pin.report.analog", (bitlash_function) analogPinReport);

  addBitlashFunction("backpack.report", (bitlash_function) backpackReport);
  addBitlashFunction("backpack.list", (bitlash_function) backpackList);

  addBitlashFunction("scout.report", (bitlash_function) scoutReport);
  addBitlashFunction("scout.isleadscout", (bitlash_function) isScoutLeadScout);
  addBitlashFunction("scout.sethqtoken", (bitlash_function) setHQToken);
  addBitlashFunction("scout.gethqtoken", (bitlash_function) getHQToken);
  addBitlashFunction("scout.delay", (bitlash_function) scoutDelay);
  addBitlashFunction("scout.daisy", (bitlash_function) daisyWipe);
  addBitlashFunction("scout.boot", (bitlash_function) wdtBoot);

  addBitlashFunction("hq.settoken", (bitlash_function) setHQToken);
  addBitlashFunction("hq.gettoken", (bitlash_function) getHQToken);
  addBitlashFunction("hq.verbose", (bitlash_function) hqVerbose);

  addBitlashFunction("events.start", (bitlash_function) startStateChangeEvents);
  addBitlashFunction("events.stop", (bitlash_function) stopStateChangeEvents);
  addBitlashFunction("events.setfreqs", (bitlash_function) setEventPeriods);
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
  if(!Scout.isLeadScout()) Shell.allReportHQ(); // lead scout reports on hq connect

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

static numvar allReport(void) {
  sp("running all reports");
  speol();
  Shell.allReportHQ();
}

static numvar allVerbose(void) {
  Scout.handler.setVerbose(getarg(1));
  isMeshVerbose = getarg(1);
  Scout.eventVerboseOutput = getarg(1);
}

void PinoccioShell::loop() {
  if (isShellEnabled) {
    runBitlash();
    key_loop(millis());
  }
}

void PinoccioShell::startShell() {
  char boot[32], i;
  isShellEnabled = true;
  initBitlash(115200);
  for(i='a';i<'z';i++)
  {
    sprintf(boot,"boot.%c",i);
    if(findscript(boot)) doCommand(boot);
  }
}

void PinoccioShell::disableShell() {
  isShellEnabled = false;
}


static NWK_DataReq_t pingDataReq;
static NWK_DataReq_t sendDataReq;
static bool sendDataReqBusy;
static int tempHigh = 0, tempLow = 0;

/****************************\
*      BUILT-IN HANDLERS    *
\****************************/
static char *tempReportHQ(void)
{
  static char report[100];
  int temp = Scout.getTemperature();
  if(temp > tempHigh) tempHigh = temp;
  if(!tempLow || temp < tempLow) tempLow = temp;
  sprintf(report,"[%d,[%d,%d,%d],[%d,%d,%d]]",key_map("temp",0),
          key_map("current",0),key_map("high",0),key_map("low",0),
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
  int free_mem;
  int uptime = millis();
  // free memory based on http://forum.pololu.com/viewtopic.php?f=10&t=989&view=unread#p4218
  sprintf(report,"[%d,[%d,%d,%d],[%d,%d,%d]]",key_map("uptime",0),
          key_map("millis",0),key_map("free",0),key_map("random",0),
          uptime,
          ((int)&free_mem) - ((int)&__bss_end),
          (int)random());
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
static numvar keyMap(void)
{
  static char num[8];
  if(isstringarg(1))
  {
    return key_map((char*)getstringarg(1), 0);
  }
  snprintf(num,8,"%d",getarg(1));
  return key_map(num, 0);
}

static numvar keyPrint(void)
{
  char *key = key_get(getarg(1));
  if(!key) return 0;
  speol(key);
}

static numvar keyNumber(void)
{
  char *key = key_get(getarg(1));
  if(!key) return 0;
  return atoi(key);
}

static numvar keySave(void)
{
  char cmd[42], *var;
  if(getarg(0) != 2 || !isstringarg(1)) return 0;
  var = (char*)getstringarg(1);
  sprintf(cmd,"function boot.%s {%s=key(\"%s\");}",var,var,key_get(getarg(2)));
  doCommand(cmd);
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
}

static char *powerReportHQ(void) {
  static char report[100];
  sprintf(report,"[%d,[%d,%d,%d,%d],[%d,%d,%s,%s]]",key_map("power",0),
          key_map("battery",0),key_map("voltage",0),key_map("charging",0),key_map("vcc",0),
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
  sprintf(report,"[%d,[%d,%d],[[%d,%d,%d],[%d,%d,%d]]]",key_map("led",0),
          key_map("led",0),key_map("torch",0),
          RgbLed.getRedValue(),RgbLed.getGreenValue(),RgbLed.getBlueValue(),
          RgbLed.getRedTorchValue(),RgbLed.getGreenTorchValue(),RgbLed.getBlueTorchValue());
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
}

static numvar ledOff(void) {
  RgbLed.turnOff();
}

static numvar ledRed(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkRed(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkRed(getarg(1));
  } else {
    RgbLed.red();
  }
}

static numvar ledGreen(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkGreen(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkGreen(getarg(1));
  } else {
    RgbLed.green();
  }
}

static numvar ledBlue(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkBlue(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkBlue(getarg(1));
  } else {
    RgbLed.blue();
  }
}

static numvar ledCyan(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkCyan(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkCyan(getarg(1));
  } else {
    RgbLed.cyan();
  }
}

static numvar ledPurple(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkPurple(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkPurple(getarg(1));
  } else {
    RgbLed.purple();
  }
}

static numvar ledMagenta(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkMagenta(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkMagenta(getarg(1));
  } else {
    RgbLed.magenta();
  }
}

static numvar ledYellow(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkYellow(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkYellow(getarg(1));
  } else {
    RgbLed.yellow();
  }
}

static numvar ledOrange(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkOrange(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkOrange(getarg(1));
  } else {
    RgbLed.orange();
  }
}

static numvar ledWhite(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkWhite(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkWhite(getarg(1));
  } else {
    RgbLed.white();
  }
}

static numvar ledGetHex(void) {
  char hex[8];
  sprintf(hex,"%02x%02x%02x", RgbLed.getRedValue(), RgbLed.getGreenValue(), RgbLed.getBlueValue());
  return key_map(hex, millis());
}

static numvar ledSetHex(void) {
  if (getarg(1)) {
    if (isstringarg(1)) {
      RgbLed.setHex((char *)getarg(1));
    } else {
      RgbLed.setHex(key_get(getarg(1)));
    }
    return true;
  } else {
    return false;
  }
}

static numvar ledSetRgb(void) {
  RgbLed.setColor(getarg(1), getarg(2), getarg(3));
}

static numvar ledSaveTorch(void) {
  RgbLed.saveTorch(getarg(1), getarg(2), getarg(3));
}

static numvar ledTorch(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkTorch(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkTorch(getarg(1));
  } else {
    RgbLed.setTorch();
  }
}

static numvar ledReport(void) {
  speol(ledReportHQ());
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
}

static numvar meshSetPower(void) {
  Scout.meshSetPower(getarg(1));
}

static numvar meshSetDataRate(void) {
  Scout.meshSetDataRate(getarg(1));
}

static numvar meshSetKey(void) {
  Scout.meshSetSecurityKey((const char *)getstringarg(1));
}

static numvar meshResetKey(void) {
  Scout.meshResetSecurityKey();
}

static numvar meshJoinGroup(void) {
  Scout.meshJoinGroup(getarg(1));
}

static numvar meshLeaveGroup(void) {
  Scout.meshLeaveGroup(getarg(1));
}

static numvar meshIsInGroup(void) {
  return Scout.meshIsInGroup(getarg(1));
}

static numvar meshPing(void) {
  pingScout(getarg(1));
}

static numvar meshPingGroup(void) {
  pingGroup(getarg(1));
}

char *arg2array(int ver, char *msg)
{
  int i;
  int args = getarg(0);
  if(args > 8) args = 8;
  sprintf(msg,"[%d,",ver);
  for(i=2; i <= args; i++)
  {
    int key = (isstringarg(i))?key_map((char*)getstringarg(i), 0):getarg(i);
    sprintf(msg+strlen(msg),"\"%s\",",key_get(key));
  }
  sprintf(msg+(strlen(msg)-1),"]");
  return msg;
}

static numvar meshSend(void) {
  char msg[100];
  if (!getarg(0)) {
    return false;
  }
  Serial.println(getarg(1));
  Serial.println(arg2array(1,msg));
  sendMessage(getarg(1), arg2array(1, msg));
  return true;
}

static numvar meshAnnounce(void) {
  char msg[100];
  if(!getarg(0)) return false;
  Scout.handler.announce(getarg(1), arg2array(1,msg));
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
}

static char *meshReportHQ(void) {
  static char report[100], c;
  int count = 0;
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i < NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) continue;
    count++;
  }
  sprintf(report,"[%d,[%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,\"",key_map("mesh",0),
          key_map("scoutid",0),key_map("troopid",0),key_map("routes",0),key_map("channel",0),key_map("rate",0),key_map("power",0),
          Scout.getAddress(),
          Scout.getPanId(),
          count,
          Scout.getChannel());
  const char *kbString = Scout.getDataRatekbps();
  while((c = pgm_read_byte(kbString++))) {
    sprintf(report+strlen(report),"%c",c);
  }
  sprintf(report+strlen(report),"\",\"");
  const char *dbString = Scout.getTxPowerDb();
  while((c = pgm_read_byte(dbString++))) {
    sprintf(report+strlen(report),"%c",c);
  }
  sprintf(report+strlen(report),"\"]]");
  return Scout.handler.report(report);
}

static numvar meshReport(void) {
  speol(meshReportHQ());
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
}

/****************************\
*        I/O HANDLERS       *
\****************************/
static char *digitalPinReportHQ(void) {
  static char report[80];
  sprintf(report,"[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d]]]",key_map("digital",0),
          key_map("mode",0),key_map("state",0),
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
  sprintf(report,"[%d,[%d,%d],[[%d,%d,%d,%d,%d,%d,%d,%d],[%d,%d,%d,%d,%d,%d,%d,%d]]]",key_map("analog",0),
          key_map("mode",0),key_map("state",0),
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
  Scout.handler.report(report);
}

static numvar pinOn(void) {
  uint8_t pin = getarg(1);
  if (Scout.isDigitalPin(pin)) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    analogPinReportHQ();
  }
}

static numvar pinOff(void) {
  uint8_t pin = getarg(1);
  if (Scout.isDigitalPin(pin)) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    analogPinReportHQ();
  }
}

static numvar pinMakeInput(void) {
  uint8_t pin = getarg(1);
  pinMode(pin, INPUT);
  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
}

static numvar pinMakeInputPullup(void) {
  uint8_t pin = getarg(1);
  pinMode(pin, INPUT_PULLUP);
  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
}

static numvar pinMakeOutput(void) {
  uint8_t pin = getarg(1);
  pinMode(pin, OUTPUT);
  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
}

static numvar pinSetMode(void) {
  uint8_t pin = getarg(1);
  pinMode(pin, getarg(2));
  if (Scout.isDigitalPin(pin)) {
    digitalPinReportHQ();
  }
  if (Scout.isAnalogPin(pin)) {
    analogPinReportHQ();
  }
}

static numvar pinRead(void) {
  int i;
  if (getarg(0) == 2) {
    i = analogRead(getarg(1));
  } else {
    i = digitalRead(getarg(1));
  }
  //sp(i);
  return i;
}

static numvar pinWrite(void) {
  // TODO: set a PWM pin's value from 0 - 255
  digitalPinReportHQ();
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

/****************************\
*     BACKPACK HANDLERS     *
\****************************/

static char *backpackReportHQ(void) {
  static char report[100];
  int comma = 0;
  sprintf(report,"[%d,[%d],[[",key_map("backpacks",0),key_map("list",0));
  for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
    for (uint8_t j = 0; j < UNIQUE_ID_LENGTH; ++j) {
      // TODO this isn't correct, dunno what to do here
      sprintf(report+strlen(report),"%s%d",comma++?",":"",Scout.bp.slave_ids[i][j]);
    }
  }
  sprintf(report+strlen(report),"]]]");
  return Scout.handler.report(report);
}

static numvar backpackReport(void) {
  sp(backpackReportHQ());
  speol();
}

static numvar backpackList(void) {
  if (Scout.bp.num_slaves == 0) {
    Serial.println("No backpacks found");
  } else {
    for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
      for (uint8_t j = 0; j < UNIQUE_ID_LENGTH; ++j) {
        if (Scout.bp.slave_ids[i][j] < 0x10) {
          Serial.print('0');
        }
        Serial.print(Scout.bp.slave_ids[i][j], HEX);
      }
      Serial.println();
    }
  }
  return 0;
}

/****************************\
 *   SCOUT REPORT HANDLERS  *
\****************************/

static char *scoutReportHQ(void) {
  static char report[100];
  sprintf(report,"[%d,[%d,%d,%d,%d,%d,%d],[%s,%d,%d,%d,%ld,%ld]]", key_map("scout",0),
           key_map("lead",0), key_map("version",0), key_map("hardware",0), key_map("family",0),  key_map("serial",0), key_map("build",0),
          Scout.isLeadScout()?"true":"false",
          (int)Scout.getEEPROMVersion(),
          (int)Scout.getHwVersion(),
          Scout.getHwFamily(),
          Scout.getHwSerial(),
          PINOCCIO_BUILD);
  return Scout.handler.report(report);
}

static numvar scoutReport(void) {
  speol(scoutReportHQ());
}

static numvar isScoutLeadScout(void) {
  speol(Scout.isLeadScout() ? 1 : 0);
  return Scout.isLeadScout();
}

static numvar setHQToken(void) {
  Pinoccio.setHQToken((const char *)getstringarg(1));
}

static numvar getHQToken(void) {
  char token[33];
  Pinoccio.getHQToken((char *)token);
  token[32] = 0;
  speol(token);
}

static numvar scoutDelay(void) {
  Scout.delay(getarg(1));
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
  sprintf(report,"[%d,[%d],[\"bye\"]]",key_map("daisy",0),key_map("dave",0));
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
    Scout.meshSetRadio(0,0x0000);
    Scout.resetHQToken();

    // so long, and thanks for all the fish!
    doCommand("rm *");
    doCommand("scout.boot");
  }
}

static numvar wdtBoot(void) {
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
}

/****************************\
 *        HQ HANDLERS       *
\****************************/

static numvar hqVerbose(void) {
  Scout.handler.setVerbose(getarg(1));
}

/****************************\
 *      EVENT HANDLERS      *
\****************************/

static numvar startStateChangeEvents(void) {
  Scout.startDigitalStateChangeEvents();
  Scout.startAnalogStateChangeEvents();
}

static numvar stopStateChangeEvents(void) {
  Scout.stopDigitalStateChangeEvents();
  Scout.stopAnalogStateChangeEvents();
}

static numvar setEventPeriods(void) {
  Scout.setStateChangeEventPeriods(getarg(1), getarg(2), getarg(3));
}

static numvar setEventVerbose(void) {
  Scout.eventVerboseOutput = getarg(1);
}

/****************************\
 *   SCOUT.WIFI.HANDLERS    *
\****************************/

static char *wifiReportHQ(void) {
  static char report[100];
  // TODO real wifi status/version
  sprintf(report,"[%d,[%d,%d,%d],[%d,%s,%s]]",key_map("wifi",0),
          key_map("version",0),key_map("connected",0),key_map("hq",0),
          0,
          Scout.wifi.isAPConnected()?"true":"false",
          Scout.wifi.isHQConnected()?"true":"false");
  return Scout.handler.report(report);
}

static numvar wifiReport(void) {
  speol(wifiReportHQ());
}

static numvar wifiStatus(void) {
  if (getarg(0) > 0 && getarg(1) == 1) {
    Scout.wifi.printProfiles(Serial);
  } else {
    Serial.print("Wi-Fi Versions: ");
    Scout.wifi.printFirmwareVersions(Serial);
    Scout.wifi.printCurrentNetworkStatus(Serial);
  }
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
}

static numvar wifiDhcp(void) {
  const char *host = (getarg(0) >= 1 ? (const char*)getstringarg(1) : NULL);

  if (!Scout.wifi.wifiDhcp(host)) {
    Serial.println("Error: saving Scout.wifi.configuration data failed");
  }
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
}

static numvar wifiPing(void) {
  if (!Scout.wifi.ping(Serial, (const char *)getstringarg(1))) {
     Serial.println("Error: Wi-Fi ping command failed");
  }
}

static numvar wifiDNSLookup(void) {
  if (!Scout.wifi.dnsLookup(Serial, (const char *)getstringarg(1))) {
     Serial.println("Error: Wi-Fi DNS lookup command failed");
  }
}

static numvar wifiGetTime(void) {
  if (!Scout.wifi.printTime(Serial)) {
     Serial.println("Error: Wi-Fi NTP time lookup command failed");
  }
}

static numvar wifiSleep(void) {
  if (!Scout.wifi.goToSleep()) {
     Serial.println("Error: Wi-Fi sleep command failed");
  }
}

static numvar wifiWakeup(void) {
  if (!Scout.wifi.wakeUp()) {
     Serial.println("Error: Wi-Fi wakeup command failed");
  }
}

static numvar wifiVerbose(void) {
  // TODO
}

/****************************\
 *     HELPER FUNCTIONS     *
\****************************/
static void pingScout(int address) {
  pingDataReq.dstAddr = address;
  char *ping = "ping";

  pingDataReq.dstEndpoint = 1;
  pingDataReq.srcEndpoint = 1;
  pingDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  pingDataReq.data = (byte *)ping;
  pingDataReq.size = strlen(ping);
  pingDataReq.confirm = pingConfirm;
  NWK_DataReq(&pingDataReq);
  //RgbLed.blinkCyan(200);

  Serial.print("PING ");
  Serial.println(address);
}

static void pingGroup(int address) {
  pingDataReq.dstAddr = address;
  char *ping = "ping";

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

  if(strlen(data) <3 || data[0] != '[') return false;

  // parse the array payload into keys, [1,"foo","bar"]
  key_load(data,keys,millis());

  // generate callback

  sprintf(buf,"event.message");
  if (findscript(buf)) {
    sprintf(buf,"event.message(%d",ind->srcAddr);
    for(int i=2;i<=keys[0];i++) sprintf(buf+strlen(buf),",%d",keys[i]);
    sprintf(buf+strlen(buf),")");
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
//  RgbLed.blinkCyan(200);

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
    sprintf(buf,"event.ack(%d,%d)",req->dstAddr,(req->status == NWK_SUCCESS_STATUS)?req->control:0);
    doCommand(buf);
  }
}


/****************************\
 *      EVENT HANDLERS      *
\****************************/
static void digitalPinEventHandler(uint8_t pin, uint8_t value) {
  uint32_t time = millis();
  char buf[32];

  digitalPinReportHQ();
  if (findscript("event.digital")) {
    String callback = "event.digital(" + String(pin) + "," + String(value) + ")";
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
  sprintf(buf,"event.digital%d",pin);
  if (findscript(buf)) {
    sprintf(buf,"event.digital%d(%d)",pin,value);
    doCommand(buf);
  }
  // simplified button trigger
  if(value == 0)
  {
    sprintf(buf,"event.button%d",pin);
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

static void analogPinEventHandler(uint8_t pin, uint16_t value) {
  uint32_t time = millis();
  char buf[32];

  analogPinReportHQ();
  if (findscript("event.analog")) {
    String callback = "event.analog(" + String(pin) + "," + String(value) + ")";
    char buf[32];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
  sprintf(buf,"event.analog%d",pin);
  if (findscript(buf)) {
    sprintf(buf,"event.analog%d(%d)",pin,value);
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
  Serial.write(b); // cc to serial
  return;

  /* this isn't used anymore, but might be useful in the future?
  static char buf[101];
  static int offset = 0;
  if (b == '\r') {
    return; // skip CR
  }

  // newline or max len announces and resets
  if (b == '\n' || offset == 100) {
    char *message;
    int chan;
    buf[offset] = 0;
    message = strchr(buf, ' ');

    // any lines looking like "CHAN:4 message" will send "message" to channel 4
    if (strncmp("CH4N:", buf, 5) == 0 && message) {
      *message = 0;
      message++;
      chan = atoi(buf+5);
      if (chan) {
        Scout.handler.announce(chan, message);
      }
    }
    offset=0;
    return;
  }

  if (b == '"') {
    buf[offset++] = '\\';
    b = '"';
  }

  buf[offset] = b;
  offset++;
  */
}

void bitlashBuffer(byte b) {
  int len;

  Serial.write(b); // cc to serial
  if (b == '\r') {
    return; // skip CR
  }

  len = strlen(Shell.bitlashOutput);
  Shell.bitlashOutput = (char*)realloc(Shell.bitlashOutput, len+3); // up to 2 bytes w/ escaping, and the null term

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
