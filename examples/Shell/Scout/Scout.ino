#include "Scout.h"

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
  addBitlashFunction("led.magenta", (bitlash_function) ledMagenta);
  addBitlashFunction("led.yellow", (bitlash_function) ledYellow);
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
}

numvar disableBackpackVcc(void) {
  Scout.disableBackpackVcc();
}

numvar goToSleep(void) {
  // TODO: not implemented yet
  //Pinoccio.goToSleep(getarg(1));
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

numvar ledMagenta(void) {
  RgbLed.magenta();
}

numvar ledYellow(void) {
  RgbLed.yellow();
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
  Pinoccio.meshSetRadio(getarg(1), getarg(2), getarg(3));
}

numvar meshSetKey(void) {
  Pinoccio.meshSetSecurityKey((const char *)getstringarg(1));
}

numvar meshRemoteRun(void) {
  // TODO: run a function that's defined on another scout
  // ie: meshRemoteRun(scoutId, "remoteFunctionName");
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
  return true;
}

numvar meshReport(void) {
  // TODO: return JSON formatted report of radio details
  // ie: {"id":34,"pid":1,"ch":26,"sec":true}
  return true;
}

/****************************\
*        I/O HANDLERS       *
\****************************/
numvar pinOn(void) {
  // TODO: turn pin on
  return true;
}

numvar pinOff(void) {
  // TODO: turn pin off
  return true;
}

numvar pinMakeInput(void) {
  // TODO: make pin an input
  return true;
}

numvar pinMakeOutput(void) {
  // TODO: make pin an ouptut
  return true;
}

numvar pinRead(void) {
  // TODO: return JSON formatted status of a single pin
  return true;
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