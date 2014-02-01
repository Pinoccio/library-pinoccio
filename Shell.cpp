#include "Shell.h"
#include "Scout.h"
#include "bitlash.h"
#include "src/bitlash.h"

PinoccioShell Shell;

PinoccioShell::PinoccioShell() {
  isShellEnabled = true;
}

PinoccioShell::~PinoccioShell() { }

void PinoccioShell::setup() {
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

  addBitlashFunction("temperature", (bitlash_function) getTemperature);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);
  addBitlashFunction("uptime", (bitlash_function) uptimeReport);
  addBitlashFunction("report", (bitlash_function) allReport);
  addBitlashFunction("verbose", (bitlash_function) allVerbose);

  addBitlashFunction("led.blink", (bitlash_function) ledBlink);
  addBitlashFunction("led.blinktorch", (bitlash_function) ledBlinkTorch);
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

  if (Scout.isLeadScout()) {
    addBitlashFunction("wifi.report", (bitlash_function) wifiReport);
    addBitlashFunction("wifi.status", (bitlash_function) wifiStatus);
    addBitlashFunction("wifi.list", (bitlash_function) wifiList);
    addBitlashFunction("wifi.config", (bitlash_function) wifiConfig);
    addBitlashFunction("wifi.dhcp", (bitlash_function) wifiDhcp);
    addBitlashFunction("wifi.static", (bitlash_function) wifiStatic);
    addBitlashFunction("wifi.reassociate", (bitlash_function) wifiReassociate);
    addBitlashFunction("wifi.command", (bitlash_function) wifiCommand);
    addBitlashFunction("wifi.ping", (bitlash_function) wifiPing);
    addBitlashFunction("wifi.dnslookup", (bitlash_function) wifiDNSLookup);
    addBitlashFunction("wifi.gettime", (bitlash_function) wifiGetTime);
    addBitlashFunction("wifi.sleep", (bitlash_function) wifiSleep);
    addBitlashFunction("wifi.wakeup", (bitlash_function) wifiWakeup);
    addBitlashFunction("wifi.verbose", (bitlash_function) wifiVerbose);
  } else {
    // bootup reporting
    Shell.allReportHQ();
  }

  Scout.meshListen(1, receiveMessage);

  // set up event handlers
  Scout.digitalPinEventHandler = digitalPinEventHandler;
  Scout.analogPinEventHandler = analogPinEventHandler;
  Scout.batteryPercentageEventHandler = batteryPercentageEventHandler;
  Scout.batteryVoltageEventHandler = batteryVoltageEventHandler;
  Scout.batteryChargingEventHandler = batteryChargingEventHandler;
  Scout.batteryAlarmTriggeredEventHandler = batteryAlarmTriggeredEventHandler;
  Scout.temperatureEventHandler = temperatureEventHandler;

  if (isShellEnabled) {
    startShell();
  } else {
    Serial.begin(115200);
  }
}

static bool isMeshVerbose;

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
  }
}

void PinoccioShell::startShell() {
  isShellEnabled = true;
  initBitlash(115200);
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
  sprintf(report,"{\"_\":\"tmp\",\"t\":%d,\"h\":%d,\"l\":%d}",temp,tempHigh,tempLow);
  Scout.handler.announce(0xBEEF, report);
  return report;
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
  sprintf(report,"{\"_\":\"u\",\"m\":%d,\"f\":%d,\"r\":%d}",uptime,((int)&free_mem) - ((int)&__bss_end),random());
  Scout.handler.announce(0xBEEF, report);
  return report;
}

static numvar uptimeReport(void) {
  uptimeReportHQ();
  sp(millis());
  speol();
  return true;
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
  sprintf(report,"{\"_\":\"pwr\",\"p\":%d,\"v\":%d,\"c\":%s,\"vcc\":%s,\"a\":%s}",Scout.getBatteryPercentage(),(int)Scout.getBatteryVoltage(),Scout.isBatteryCharging()?"true":"false",Scout.isBackpackVccEnabled()?"true":"false", Scout.isBatteryAlarmTriggered()?"true":"false");
  Scout.handler.announce(0xBEEF, report);
  return report;
}

static numvar powerReport(void) {
  speol(powerReportHQ());
  return true;
}

/****************************\
*      RGB LED HANDLERS     *
\****************************/
static numvar ledBlink(void) {
  if (getarg(0) == 5) {
    RgbLed.blinkColor(getarg(1), getarg(2), getarg(3), getarg(4), getarg(5));
  } else if (getarg(0) == 4) {
    RgbLed.blinkColor(getarg(1), getarg(2), getarg(3), getarg(4));
  } else {
    RgbLed.blinkColor(getarg(1), getarg(2), getarg(3));
  }
}

static numvar ledBlinkTorch(void) {
  if (getarg(0) == 1) {
    RgbLed.blinkTorch(getarg(1));
  } else {
    RgbLed.blinkTorch();
  }
}

static char *ledReportHQ(void) {
  static char report[100];
  sprintf(report,"{\"_\":\"led\",\"l\":[%d,%d,%d],\"t\":[%d,%d,%d]}",RgbLed.getRedValue(),RgbLed.getGreenValue(),RgbLed.getBlueValue(),RgbLed.getRedTorchValue(),RgbLed.getGreenTorchValue(),RgbLed.getBlueTorchValue());
  Scout.handler.announce(0xBEEF, report);
  return report;
}

static numvar ledOff(void) {
  RgbLed.turnOff();
  ledReportHQ();
}

static numvar ledRed(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkRed(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkRed(getarg(1));
  } else {
    RgbLed.red();
  }
  ledReportHQ();
}

static numvar ledGreen(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkGreen(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkGreen(getarg(1));
  } else {
    RgbLed.green();
  }
  ledReportHQ();
}

static numvar ledBlue(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkBlue(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkBlue(getarg(1));
  } else {
    RgbLed.blue();
  }
  ledReportHQ();
}

static numvar ledCyan(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkCyan(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkCyan(getarg(1));
  } else {
    RgbLed.cyan();
  }
  ledReportHQ();
}

static numvar ledPurple(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkPurple(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkPurple(getarg(1));
  } else {
    RgbLed.purple();
  }
  ledReportHQ();
}

static numvar ledMagenta(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkMagenta(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkMagenta(getarg(1));
  } else {
    RgbLed.magenta();
  }
  ledReportHQ();
}

static numvar ledYellow(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkYellow(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkYellow(getarg(1));
  } else {
    RgbLed.yellow();
  }
  ledReportHQ();
}

static numvar ledOrange(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkOrange(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkOrange(getarg(1));
  } else {
    RgbLed.orange();
  }
  ledReportHQ();
}

static numvar ledWhite(void) {
  if (getarg(0) == 2) {
    RgbLed.blinkWhite(getarg(1), getarg(2));
  } else if (getarg(0) == 1) {
    RgbLed.blinkWhite(getarg(1));
  } else {
    RgbLed.white();
  }
  ledReportHQ();
}

static numvar ledGetHex(void) {
  char *buf;
  if (!isstringarg(1)) return false;
  buf = (char*)malloc(strlen((char*)getstringarg(1))+16);
  sprintf(buf,"%s(\"%02x%02x%02x\")",(char*)getstringarg(1),RgbLed.getRedValue(),RgbLed.getGreenValue(),RgbLed.getBlueValue());
//  sp("calling ");
//  speol(buf);
  doCommand(buf);
  free(buf);
  return true;
}

static numvar ledSetHex(void) {
  if (getarg(1)) {
    RgbLed.setHex((char *)getarg(1));
    ledReportHQ();
    return true;
  } else {
    return false;
  }
}

static numvar ledSetRgb(void) {
  RgbLed.setColor(getarg(1), getarg(2), getarg(3));
  ledReportHQ();
}

static numvar ledSaveTorch(void) {
  RgbLed.saveTorch(getarg(1), getarg(2), getarg(3));
  ledReportHQ();
}

static numvar ledTorch(void) {
  RgbLed.setTorch();
  ledReportHQ();
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

static numvar meshSend(void) {
  bool getAck = true;
  if (getarg(0) == 3) {
    bool getAck = getarg(3);
  }
  sendMessage(getarg(1), (char *)getstringarg(2), getAck);
}

static numvar meshAnnounce(void) {
  Scout.handler.announce(getarg(1), (char*)getarg(2));
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

  sprintf(report,"{\"_\":\"rf\",\"id\":%d,\"p\":%d,\"t\":%d,\"c\":%d,\"x\":\"",Scout.getAddress(),Scout.getPanId(),count,Scout.getChannel());
  const char *kbString = Scout.getDataRatekbps();
  while((c = pgm_read_byte(kbString++))) {
    sprintf(report+strlen(report),"%c",c);
  }
  sprintf(report+strlen(report),"\",\"w\":\"");
  const char *dbString = Scout.getTxPowerDb();
  while((c = pgm_read_byte(dbString++))) {
    sprintf(report+strlen(report),"%c",c);
  }
  sprintf(report+strlen(report),"\"}");
  Scout.handler.announce(0xBEEF, report);
  return report;
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
  sprintf(report,"{\"_\":\"pind\",\"m\":[%d,%d,%d,%d,%d,%d,%d],\"v\":[%d,%d,%d,%d,%d,%d,%d]}",
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
  Scout.handler.announce(0xBEEF, report);
  return report;
}

static char *analogPinReportHQ(void) {
  static char report[80];
  sprintf(report,"{\"_\":\"pina\",\"m\":[%d,%d,%d,%d,%d,%d,%d,%d],\"v\":[%d,%d,%d,%d,%d,%d,%d,%d]}",
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
  Scout.handler.announce(0xBEEF, report);
  return report;
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
  sprintf(report,"{\"_\":\"bps\",\"a\":[");
  for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
    for (uint8_t j = 0; j < UNIQUE_ID_LENGTH; ++j) {
      // TODO this isn't correct, dunno what to do here
      sprintf(report+strlen(report),"%s%d",comma++?",":"",Scout.bp.slave_ids[i][j]);
    }
  }
  sprintf(report+strlen(report),"]}");
  Scout.handler.announce(0xBEEF, report);
  return report;
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
        Serial.print(Scout.bp.slave_ids[i][j]);
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
  sprintf(report,"{\"_\":\"s\",\"e\":%d,\"hv\":%d,\"hf\":%d,\"hs\":%d,\"b\":%ld}",(int)Scout.getEEPROMVersion(),(int)Scout.getHwVersion(),Scout.getHwFamily(),Scout.getHwSerial(),PINOCCIO_BUILD);
  Scout.handler.announce(0xBEEF, report);
  return report;
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
  static char report[] = "{\"_\":\"daisy\",\"m\":\"Ok, terminating. Goodbye Dave\"}";

  if (Scout.isLeadScout()) {
    if (!Scout.wifi.runDirectCommand(Serial, "AT&F")) {
       Serial.println("Error: Wi-Fi direct command failed");
       ret = false;
    }
    if (!Scout.wifi.runDirectCommand(Serial, "AT&W0")) {
       Serial.println("Error: Wi-Fi direct command failed");
       ret = false;
    }
  }

  if (ret == true) {
    Scout.handler.announce(0xBEEF, report);
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
 *       Scout.wifi.HANDLERS      *
\****************************/

static char *wifiReportHQ(void) {
  static char report[100];
  // TODO real wifi status/version
  sprintf(report,"{\"_\":\"bp\",\"b\":\"wifi\",\"v\":%d,\"c\":%s}",0,"true");
  Scout.handler.announce(0xBEEF, report);
  return report;
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
    Scout.wifi.printAPs(Serial);
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
  if (isMeshVerbose) {
    Serial.print("Received message - ");
    Serial.print("lqi: ");
    Serial.print(ind->lqi, DEC);

    Serial.print("  ");

    Serial.print("rssi: ");
    Serial.print(abs(ind->rssi), DEC);
    Serial.print("  ");

    Serial.print("data: ");
    for (int i=0; i<ind->size; i++) {
      Serial.print(ind->data[i], DEC);
    }
    Serial.println("");
  }

  NWK_SetAckControl(abs(ind->rssi));

  // run the Bitlash callback function, if defined
  char *callback = "event.message";
  if (findscript(callback)) {
    doCommand(callback);
  }
  return true;
}

static void sendMessage(int address, char *data, bool getAck) {
  if (sendDataReqBusy) {
    return;
  }

  sendDataReq.dstAddr = address;

  sendDataReq.dstEndpoint = 1;
  sendDataReq.srcEndpoint = 1;
  if (getAck) {
    sendDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  } else {
    sendDataReq.options = NWK_OPT_ENABLE_SECURITY;
  }
  sendDataReq.data = (byte *)data;
  sendDataReq.size = strlen(data);
  sendDataReq.confirm = sendConfirm;
  NWK_DataReq(&sendDataReq);
  RgbLed.blinkCyan(200);

  sendDataReqBusy = true;

  if (isMeshVerbose) {
    Serial.print("Sent message to Scout ");
    Serial.println(address);
  }
}

static void sendConfirm(NWK_DataReq_t *req) {
   sendDataReqBusy = false;

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

static void batteryAlarmTriggeredEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.batteryalarm")) {
    String callback = "event.batteryalarm(" + String(value) + ")";
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

void bitlashFilter(byte b) {
  static char buf[101];
  static int offset = 0;

  Serial.write(b); // cc to serial
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
