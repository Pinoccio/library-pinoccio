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
  addBitlashFunction("mesh.announce", (bitlash_function) meshAnnounce);

  addBitlashFunction("temperature", (bitlash_function) getTemperature);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);

  addBitlashFunction("led.blink", (bitlash_function) ledBlink);
  addBitlashFunction("led.blinktorch", (bitlash_function) ledBlinkTorch);
  addBitlashFunction("led.enableblink", (bitlash_function) ledEnableContinuousBlink);
  addBitlashFunction("led.disableblink", (bitlash_function) ledDisableContinuousBlink);
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
  addBitlashFunction("led.hexvalue", (bitlash_function) ledSetHexValue);
  addBitlashFunction("led.setrgbvalue", (bitlash_function) ledSetRgb);
  addBitlashFunction("led.savetorch", (bitlash_function) ledSaveTorch);
  addBitlashFunction("led.report", (bitlash_function) ledReport);

  addBitlashFunction("pin.on", (bitlash_function) pinOn);
  addBitlashFunction("pin.off", (bitlash_function) pinOff);
  addBitlashFunction("pin.makeinput", (bitlash_function) pinMakeInput);
  addBitlashFunction("pin.makeinputup", (bitlash_function) pinMakeInputPullup);
  addBitlashFunction("pin.makeoutput", (bitlash_function) pinMakeOutput);
  addBitlashFunction("pin.read", (bitlash_function) pinRead);
  addBitlashFunction("pin.write", (bitlash_function) pinWrite);
  addBitlashFunction("pin.report", (bitlash_function) pinReport);

  addBitlashFunction("backpack.report", (bitlash_function) backpackReport);
  addBitlashFunction("backpack.list", (bitlash_function) backpackList);

  addBitlashFunction("scout.report", (bitlash_function) scoutReport);
  addBitlashFunction("scout.isleadscout", (bitlash_function) isScoutLeadScout);
  addBitlashFunction("scout.sethqtoken", (bitlash_function) setHQToken);
  addBitlashFunction("scout.gethqtoken", (bitlash_function) getHQToken);
  addBitlashFunction("scout.boot", (bitlash_function) wdtBoot);

  addBitlashFunction("events.start", (bitlash_function) startStateChangeEvents);
  addBitlashFunction("events.stop", (bitlash_function) stopStateChangeEvents);
  addBitlashFunction("events.setfreqs", (bitlash_function) setEventPeriods);
  addBitlashFunction("events.verbose", (bitlash_function) setEventVerbose);

  if (Scout.isLeadScout()) {
    addBitlashFunction("wifi.report", (bitlash_function) wifiReport);
    addBitlashFunction("wifi.list", (bitlash_function) wifiList);
    addBitlashFunction("wifi.config", (bitlash_function) wifiConfig);
    addBitlashFunction("wifi.command", (bitlash_function) wifiCommand);
    addBitlashFunction("wifi.ping", (bitlash_function) wifiPing);
    addBitlashFunction("wifi.dnslookup", (bitlash_function) wifiDNSLookup);
    addBitlashFunction("wifi.gettime", (bitlash_function) wifiGetTime);
    addBitlashFunction("wifi.sleep", (bitlash_function) wifiSleep);
    addBitlashFunction("wifi.wakeup", (bitlash_function) wifiWakeup);
    addBitlashFunction("wifi.verbose", (bitlash_function) wifiVerbose);
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
static bool isMeshVerbose;

/****************************\
*      BUILT-IN HANDLERS    *
\****************************/
static numvar getTemperature(void) {
  int i = Scout.getTemperature();
  sp(i);
  speol();
  return i;
}

static numvar getRandomNumber(void) {
  int i = random();
  sp(i);
  speol();
  return i;
}

/****************************\
*      POWER HANDLERS       *
\****************************/
static numvar isBatteryCharging(void) {
  int i = Scout.isBatteryCharging();
  sp(i);
  speol();
  return i;
}

static numvar getBatteryPercentage(void) {
  int i = Scout.getBatteryPercentage();
  sp(i);
  speol();
  return i;
}

static numvar getBatteryVoltage(void) {
  int i = Scout.getBatteryVoltage();
  sp(i);
  speol();
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

void powerReportHQ(void)
{
  char report[100];
  sprintf(report,"{\"_\":\"pwr\",\"p\":%d,\"v\":%d,\"c\":%s,\"vcc\":%s,\"a\":%s}",Scout.getBatteryPercentage(),(int)Scout.getBatteryVoltage(),Scout.isBatteryCharging()?"true":"false",Scout.isBackpackVccEnabled()?"true":"false", Scout.isBatteryAlarmTriggered()?"true":"false");
  Scout.handler.fieldAnnounce(0xBEEF, report);
}

static numvar powerReport(void) {
  powerReportHQ();
  sp("percent charged: ");
  sp(Scout.getBatteryPercentage());
  sp("\nvoltage: ");
  sp((int)Scout.getBatteryVoltage());
  sp("\ncharging: ");
  sp(Scout.isBatteryCharging());
  sp("\nvcc: ");
  sp(Scout.isBackpackVccEnabled());
  sp("\nalarm: ");
  sp(Scout.isBatteryAlarmTriggered());
  sp("\n");
  return true;
}

/****************************\
*      RGB LED HANDLERS     *
\****************************/
static numvar ledEnableContinuousBlink(void) {
  RgbLed.enableContinuousBlink();
}

static numvar ledDisableContinuousBlink(void) {
  RgbLed.disableContinuousBlink();
}

static numvar ledBlink(void) {
  if (getarg(0) == 4) {
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

static numvar ledOff(void) {
  RgbLed.turnOff();
}

static numvar ledRed(void) {
  RgbLed.red();
}

static numvar ledGreen(void) {
  RgbLed.green();
}

static numvar ledBlue(void) {
  RgbLed.blue();
}

static numvar ledCyan(void) {
  RgbLed.cyan();
}

static numvar ledPurple(void) {
  RgbLed.purple();
}

static numvar ledMagenta(void) {
  RgbLed.magenta();
}

static numvar ledYellow(void) {
  RgbLed.yellow();
}

static numvar ledOrange(void) {
  RgbLed.orange();
}

static numvar ledWhite(void) {
  RgbLed.white();
}

static numvar ledSetHexValue(void) {
  Serial.println((char *)getstringarg(1));
  if (isstringarg(1)) {
    RgbLed.setHex((char *)getstringarg(1));
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
  RgbLed.setTorch();
}

static numvar ledReport(void) {
  sp("{\"c\": {\"r\":");
  sp(RgbLed.getRedValue());
  sp(",\"g\":");
  sp(RgbLed.getGreenValue());
  sp(",\"b\":");
  sp(RgbLed.getBlueValue());
  sp("}, \"t\": {\"r\":");
  sp(RgbLed.getRedTorchValue());
  sp(",\"g\":");
  sp(RgbLed.getGreenTorchValue());
  sp(",\"b\":");
  sp(RgbLed.getBlueTorchValue());
  sp("}}\n");
}

/****************************\
*    MESH RADIO HANDLERS    *
\****************************/
static numvar meshConfig(void) {
  uint16_t panId = 0x4567;
  uint8_t channel = 0x1a;
  if (getarg(0) == 2) {
    panId = getarg(2);
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
  sendMessage(getarg(1), (char *)getstringarg(2), true);
}

static numvar meshAnnounce(void) {
  Scout.handler.fieldAnnounce(getarg(1), (char*)getstringarg(2));
}

static numvar meshVerbose(void) {
  isMeshVerbose = getarg(1);
}

static numvar meshReport(void) {
  // TODO: return JSON formatted report of radio details
  // ie: {"id":34,"pid":1,"ch":26,"sec":true}
  Serial.println("-- Mesh Radio Settings --");
  Serial.print(" - Address: ");
  Serial.println(Scout.getAddress());
  Serial.print(" - Pan ID: 0x");
  Serial.println(Scout.getPanId(), HEX);
  Serial.print(" - Channel: ");
  Serial.println(Scout.getChannel());
  Serial.print(" - Data Rate: ");
  // gotta read these from program memory (for SRAM savings)
  char c;
  const char *kbString = Scout.getDataRatekbps();
  while((c = pgm_read_byte(kbString++))) {
     Serial.write(c);
  }
  Serial.println();
  Serial.print(" - Tx Power: ");
  // gotta read these from program memory (for SRAM savings)
  const char *dbString = Scout.getTxPowerDb();
  while((c = pgm_read_byte(dbString++))) {
     Serial.write(c);
  }
  Serial.println();
  // TODO: loop through all NWK_GetGroups() output and print
  // Serial.print(" - In groups: ");
  //   if (Scout.meshGetGroups()) {
  //     Serial.println("Yes");
  //   } else {
  //     Serial.println("No");
  //   }
  Serial.println(" - Routing: ");
  Serial.println("|    Fixed    |  Multicast  |    Score    |    DstAdd   | NextHopAddr |    Rank     |     LQI    |");
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i < NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) {
      continue;
    }
    Serial.print("|      ");
    Serial.print(table[i].fixed);
    Serial.print("      |      ");
    Serial.print(table[i].multicast);
    Serial.print("      |      ");
    Serial.print(table[i].score);
    Serial.print("      |     0x");
    Serial.print(table[i].dstAddr, HEX);
    Serial.print("     |     0x");
    Serial.print(table[i].nextHopAddr, HEX);
    Serial.print("     |     ");
    Serial.print(table[i].rank);
    Serial.print("     |     ");
    Serial.print(table[i].lqi);
    Serial.println("    |");
  }
}

/****************************\
*        I/O HANDLERS       *
\****************************/
static numvar pinOn(void) {
  pinMode(getarg(1), OUTPUT);
  digitalWrite(getarg(1), HIGH);
}

static numvar pinOff(void) {
  pinMode(getarg(1), OUTPUT);
  digitalWrite(getarg(1), LOW);
}

static numvar pinMakeInput(void) {
  pinMode(getarg(1), INPUT);
}

static numvar pinMakeInputPullup(void) {
  pinMode(getarg(1), INPUT_PULLUP);
}

static numvar pinMakeOutput(void) {
  pinMode(getarg(1), OUTPUT);
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
  return true;
}

static numvar pinThreshold(void) {
  // TODO: create a threshold function with the following format:
  // threshold(pin, value, fnToCallIfValueLessThan, fnToCallIfValueEqual, fnToCallIfValueGreaterThan)
  return true;
}

static numvar pinReport(void) {
  // TODO: return JSON formmated report of all IO pins and their values
  sp("{\"d2\": ");
  sp(Scout.digitalPinState[0]);
  sp(", \"d3\": ");
  sp(Scout.digitalPinState[1]);
  sp(", \"d4\": ");
  sp(Scout.digitalPinState[2]);
  sp(", \"d5\": ");
  sp(Scout.digitalPinState[3]);
  sp(", \"d6\": ");
  sp(Scout.digitalPinState[4]);
  sp(", \"d7\": ");
  sp(Scout.digitalPinState[5]);
  sp(", \"d8\": ");
  sp(Scout.digitalPinState[6]);
  sp(", \"a0\": ");
  sp(Scout.analogPinState[0]);
  sp(", \"a1\": ");
  sp(Scout.analogPinState[1]);
  sp(", \"a2\": ");
  sp(Scout.analogPinState[2]);
  sp(", \"a3\": ");
  sp(Scout.analogPinState[3]);
  sp(", \"a4\": ");
  sp(Scout.analogPinState[4]);
  sp(", \"a5\": ");
  sp(Scout.analogPinState[5]);
  sp(", \"a6\": ");
  sp(Scout.analogPinState[6]);
  sp(", \"a7\": ");
  sp(Scout.analogPinState[7]);
  sp("}");
  return true;
}

/****************************\
*     BACKPACK HANDLERS     *
\****************************/
static numvar backpackReport(void) {
  sp("[{\"name\":\"wifi\",\"version\":\"1.0\"},{\"name\":\"environment\",\"version\":\"2.0\"}]");
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
static numvar scoutReport(void) {
  sp("{\"e\":");
  sp((int)Scout.getEEPROMVersion());
  sp(",\"hwv\":");
  sp((int)Scout.getHwVersion());
  sp(",\"hwf\":");
  sp((int)Scout.getHwFamily());
  sp(",\"hwid\":");
  sp((int)Scout.getHwSerial());
  sp("}");
  speol();
}

static numvar isScoutLeadScout(void) {
  sp(Scout.isLeadScout() ? 1 : 0);
  speol();
  return Scout.isLeadScout();
}

static numvar setHQToken(void) {
  Pinoccio.setHQToken((const char *)getstringarg(1));
}

static numvar getHQToken(void) {
  char token[33];
  Pinoccio.getHQToken((char *)token);
  token[32] = 0;
  sp(token);
  speol();
}

static numvar wdtBoot(void) {
  cli();
  wdt_enable(WDTO_15MS);
  while(1);
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
  Scout.setStateChangeEventPeriods(getarg(1), getarg(2));
}

static numvar setEventVerbose(void) {
  Scout.eventVerboseOutput = getarg(1);
}

/****************************\
 *       Scout.wifi.HANDLERS      *
\****************************/
static numvar wifiReport(void) {
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
  if (!Scout.wifi.autoConfig((const char *)getstringarg(1), (const char *)getstringarg(2), (const char *)getstringarg(3), getarg(4))) {
    Serial.println("Error: saving Scout.wifi.configuration data failed");
  }
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

  if (findscript("event.digital")) {
    String callback = "event.digital(" + String(pin) + "," + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print("Digital pin event handler took ");
    Serial.print(millis() - time);
    Serial.println("ms");
    Serial.println();
  }
}

static void analogPinEventHandler(uint8_t pin, uint16_t value) {
  if (findscript("event.analog")) {
    String callback = "event.analog(" + String(pin) + "," + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void batteryPercentageEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.percent")) {
    String callback = "event.percent(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void batteryVoltageEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.voltage")) {
    String callback = "event.voltage(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void batteryChargingEventHandler(uint8_t value) {
  powerReportHQ();
  if (findscript("event.charging")) {
    String callback = "event.charging(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void batteryAlarmTriggeredEventHandler(uint8_t value) {
  if (findscript("event.batteryalarm")) {
    String callback = "event.batteryalarm(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

static void temperatureEventHandler(uint8_t value) {
  if (findscript("event.temperature")) {
    String callback = "event.temperature(" + String(value) + ")";
    char buf[24];
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
        Scout.handler.fieldAnnounce(chan, message);
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
