#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

const uint16_t groupId = 0xBEEF;
static byte pingCounter = 0;
static NWK_DataReq_t appDataReq;

void setup(void) {
  Scout.setup();

  addBitlashFunction("power.charging", (bitlash_function) isBatteryCharging);
  addBitlashFunction("power.percent", (bitlash_function) getBatteryPercentage);
  addBitlashFunction("power.voltage", (bitlash_function) getBatteryVoltage);
  addBitlashFunction("power.enablevcc", (bitlash_function) enableBackpackVcc);
  addBitlashFunction("power.disablevcc", (bitlash_function) disableBackpackVcc);
  addBitlashFunction("power.sleep", (bitlash_function) goToSleep);
  addBitlashFunction("power.report", (bitlash_function) powerReport);

  addBitlashFunction("mesh.config", (bitlash_function) meshConfig);
  addBitlashFunction("mesh.key", (bitlash_function) meshSetKey);
  addBitlashFunction("mesh.joingroup", (bitlash_function) meshJoinGroup);
  addBitlashFunction("mesh.leavegroup", (bitlash_function) meshLeaveGroup);
  addBitlashFunction("mesh.ping", (bitlash_function) meshPing);
  addBitlashFunction("mesh.pinggroup", (bitlash_function) meshPingGroup);
  addBitlashFunction("mesh.remoterun", (bitlash_function) meshRemoteRun);
  addBitlashFunction("mesh.broadcastrun", (bitlash_function) meshBroadcastRun);
  addBitlashFunction("mesh.publish", (bitlash_function) meshPublish);
  addBitlashFunction("mesh.subscribe", (bitlash_function) meshSubscribe);
  addBitlashFunction("mesh.report", (bitlash_function) meshReport);

  addBitlashFunction("temperature", (bitlash_function) getTemperature);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);

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
  addBitlashFunction("led.redvalue", (bitlash_function) ledSetRedValue);
  addBitlashFunction("led.greenvalue", (bitlash_function) ledSetGreenValue);
  addBitlashFunction("led.bluevalue", (bitlash_function) ledSetBlueValue);
  addBitlashFunction("led.report", (bitlash_function) ledReport);

  addBitlashFunction("pin.on", (bitlash_function) pinOn);
  addBitlashFunction("pin.off", (bitlash_function) pinOff);
  addBitlashFunction("pin.makeinput", (bitlash_function) pinMakeInput);
  addBitlashFunction("pin.makeoutput", (bitlash_function) pinMakeOutput);
  addBitlashFunction("pin.read", (bitlash_function) pinRead);
  addBitlashFunction("pin.write", (bitlash_function) pinWrite);
  addBitlashFunction("pin.report", (bitlash_function) pinReport);

  addBitlashFunction("backpack.report", (bitlash_function) backpackReport);

  Scout.meshListen(1, receiveMessage);
}

void loop(void) {
  Scout.loop();
}

/****************************\
*      BUILT-IN HANDLERS    *
\****************************/
numvar getTemperature(void) {
  return Scout.getTemperature();
}

numvar getRandomNumber(void) {
  return Scout.getRandomNumber();
}

/****************************\
*      POWER HANDLERS       *
\****************************/
numvar isBatteryCharging(void) {
  return Scout.isBatteryCharging();
}

numvar getBatteryPercentage(void) {
  // TODO: broken, crashes board
  return Scout.getBatteryPercentage();
}

numvar getBatteryVoltage(void) {
  // TODO: broken, crashes board
  return Scout.getBatteryVoltage();
}

numvar enableBackpackVcc(void) {
  Scout.enableBackpackVcc();
  return true;
}

numvar disableBackpackVcc(void) {
  Scout.disableBackpackVcc();
  return true;
}

numvar goToSleep(void) {
  // TODO: not implemented yet
  //Scout.goToSleep(getarg(1));
}

numvar powerReport(void) {
  // TODO: return JSON formmated report of power details
  // ie: {pct:85,vlt:4.1,chg:false,vcc:true}
  return true;
}

/****************************\
*      RGB LED HANDLERS     *
\****************************/
numvar ledOff(void) {
  RgbLed.turnOff();
}

numvar ledRed(void) {
  RgbLed.red();
}

numvar ledGreen(void) {
  RgbLed.green();
}

numvar ledBlue(void) {
  RgbLed.blue();
}

numvar ledCyan(void) {
  RgbLed.cyan();
}

numvar ledPurple(void) {
  RgbLed.purple();
}

numvar ledMagenta(void) {
  RgbLed.magenta();
}

numvar ledYellow(void) {
  RgbLed.yellow();
}

numvar ledOrange(void) {
  RgbLed.orange();
}

numvar ledWhite(void) {
  RgbLed.white();
}

numvar ledSetRedValue(void) {
  RgbLed.setRedValue(getarg(1));
}

numvar ledSetGreenValue(void) {
  RgbLed.setGreenValue(getarg(1));
}

numvar ledSetBlueValue(void) {
  RgbLed.setBlueValue(getarg(1));
}

numvar ledSetHex(void) {
  // TODO: broken
  /*
  if (isstringarg(1)) {
    RgbLed.setHex(getstringarg(1));
    return true;
  } else {
    return false;
  }
  */
}

numvar ledReport(void) {
  // TODO: not working yet
  // ie: {"r":120,"g":80,"b":0}
  return printf("{\"r\":%d,\"g\":%d,\"b\":%d}\n", RgbLed.getRedValue(), RgbLed.getGreenValue(), RgbLed.getBlueValue());
}

/****************************\
*    MESH RADIO HANDLERS    *
\****************************/
numvar meshConfig(void) {
  uint16_t panId = 0x4567;
  uint8_t channel = 0x1a;
  if (getarg(0) == 2) {
    panId = getarg(2);
    channel = getarg(3);
  }
  Scout.meshSetRadio(getarg(1), panId, channel);
}

numvar meshSetKey(void) {
  Scout.meshSetSecurityKey((const char *)getstringarg(1));
}

numvar meshJoinGroup(void) {
  Scout.meshJoinGroup(groupId);
}

numvar meshLeaveGroup(void) {
  Scout.meshLeaveGroup(groupId);
}

numvar meshPing(void) {
  pingScout(getarg(1));
}

numvar meshPingGroup(void) {
  pingGroup(groupId);
}

// Run a command that's defined on another scout.  ie: meshRemoteRun(scoutId, "remote command string");
numvar meshRemoteRun(void) {
  MeshRequest request;
  request.setDstAddress(getarg(1));
  request.setDstEndpoint(1);
  request.setSrcEndpoint(1);

  if (!isstringarg(2)) {
    Serial.println("Second argument must be a valid Bitlash command (and a string)");
    return false;
  }

  if (sizeof(getarg(2)) > 100) {
    Serial.println("Size of payload cannot exceed 100 bytes");
    return false;
  }

  uint8_t size = strlen((const char *)getstringarg(2));

  Serial.println(getarg(0));
  Serial.println(getarg(1));
  Serial.println((const char *) getstringarg(2));
  Serial.println(size);

  request.setHeader(NWK_PAYLOAD_HEADER_FINAL);
  request.setPayload((byte *)getstringarg(2), size+1);

  NWK_DataReq_t* dataReq = request.getRequest();
  Serial.println("Data request");
  Serial.println(dataReq->dstAddr);
  Serial.println(dataReq->dstEndpoint);
  Serial.println(dataReq->srcEndpoint);
  Serial.println(dataReq->size);
  Serial.println(dataReq->data[0], HEX);
  for (int i=1; i<dataReq->size; i++) {
    Serial.write((const char*)dataReq->data[i]);
  }
  Scout.meshSendMessage(request);
  return true;
}

numvar meshBroadcastRun(void) {
  // TODO: run a function that's defined on on all boards in a troop
  // ie: meshRemoteRun("remoteFunctionName");
  return true;
}

numvar meshPublish(void) {
  // TODO: send a payload to a multicast address on the mesh
  // ie: meshPublish(multicastId, callbackFunctionThatReturnsPayloadToPublish, frequencyToPublish);
  return true;
}

numvar meshSubscribe(void) {
  // TODO: subscribe to/join a multicast address on the mesh
  // ie: meshSubscribe(multicastId, callbackFunctionThatIsCalledWhenMessageArrives);
  NWK_GroupAdd(getarg(1));
  if (!NWK_GroupIsMember(getarg(1))) {
    Serial.println("Error attempting to subscribe to topic");
    return false;
  }
  return true;
}

numvar meshReport(void) {
  // TODO: return JSON formatted report of radio details
  // ie: {"id":34,"pid":1,"ch":26,"sec":true}
  Serial.println("-- Mesh Radio Settings --");
  Serial.print("      Address: ");
  Serial.println(Scout.getAddress());
  Serial.print("       Pan ID: 0x");
  Serial.println(Scout.getPanId(), HEX);
  Serial.print("      Channel: ");
  Serial.println(Scout.getChannel());
  Serial.print("     Tx Power: ");
  // gotta read these from program memory (for SRAM savings)
  char c;
  const char *dbString = Scout.getTxPowerDb();
  while((c = pgm_read_byte(dbString++))) {
     Serial.write(c);
  }
  Serial.println();
// TODO
//  Serial.print("   -   Asleep: ");
//  Serial.println();
//  Serial.print("   - In group: ");
//  Serial.println();
//  Serial.print("   -  Routing: ");
//  Serial.println();

}

/****************************\
*        I/O HANDLERS       *
\****************************/
numvar pinOn(void) {
  pinMode(getarg(1), OUTPUT);
  digitalWrite(getarg(1), HIGH);
  return true;
}

numvar pinOff(void) {
  pinMode(getarg(1), OUTPUT);
  digitalWrite(getarg(1), LOW);
  return true;
}

numvar pinMakeInput(void) {
  pinMode(getarg(1), INPUT);
  return true;
}

numvar pinMakeOutput(void) {
  pinMode(getarg(1), OUTPUT);
  return true;
}

numvar pinRead(void) {
  return digitalRead(getarg(1));
}

numvar pinWrite(void) {
  // TODO: set a PWM pin's value from 0 - 255
  return true;
}

numvar pinThreshold(void) {
  // TODO: create a threshold function with the following format:
  // threshold(pin, value, fnToCallIfValueLessThan, fnToCallIfValueEqual, fnToCallIfValueGreaterThan)
  return true;
}

numvar pinReport(void) {
  // TODO: return JSON formmated report of all IO pins and their values
  return true;
}

/****************************\
*     BACKPACK HANDLERS     *
\****************************/
numvar backpackReport(void) {
  // TODO: return JSON formmated report of all backpacks attached
  return true;
}

// Helper functions
static void pingScout(int address) {
  appDataReq.dstAddr = address;

  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = pingConfirm;
  NWK_DataReq(&appDataReq);
  //RgbLed.blinkCyan(200);

  Serial.print("PING ");
  Serial.print(address);
  Serial.print(": ");

  pingCounter++;
}

static void pingGroup(int address) {
  appDataReq.dstAddr = address;

  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_MULTICAST|NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = pingConfirm;
  NWK_DataReq(&appDataReq);

  Serial.print("PING ");
  Serial.print(address);
  Serial.print(": ");

  pingCounter++;
}

static void pingConfirm(NWK_DataReq_t *req) {
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
  NWK_SetAckControl(abs(ind->rssi));

  // run the Bitlash callback function, if defined
  //doCommand("mesh.receive(...)")
  return true;
}
