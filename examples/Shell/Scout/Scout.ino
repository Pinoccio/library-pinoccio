#include "Scout.h"

void setup(void) {
  Scout.setup();

  addBitlashFunction("temperature", (bitlash_function) getTemperature);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);
  
  addBitlashFunction("power.charging", (bitlash_function) isBatteryCharging);
  addBitlashFunction("power.percent", (bitlash_function) getBatteryPercentage);
  addBitlashFunction("power.voltage", (bitlash_function) getBatteryVoltage);
  addBitlashFunction("power.enablevcc", (bitlash_function) enableBackpackVcc);
  addBitlashFunction("power.disablevcc", (bitlash_function) disableBackpackVcc);
  addBitlashFunction("power.sleep", (bitlash_function) goToSleep);
  
  addBitlashFunction("led.enable", (bitlash_function) ledEnable);
  addBitlashFunction("led.disable", (bitlash_function) ledDisable);
  addBitlashFunction("led.enabled", (bitlash_function) ledIsEnabled);
  addBitlashFunction("led.off", (bitlash_function) ledOff);
  addBitlashFunction("led.red", (bitlash_function) ledRed);
  addBitlashFunction("led.green", (bitlash_function) ledGreen);
  addBitlashFunction("led.blue", (bitlash_function) ledBlue);
  addBitlashFunction("led.cyan", (bitlash_function) ledCyan);
  addBitlashFunction("led.magenta", (bitlash_function) ledMagenta);
  addBitlashFunction("led.yellow", (bitlash_function) ledYellow);
  addBitlashFunction("led.white", (bitlash_function) ledWhite);
  addBitlashFunction("led.blinkred", (bitlash_function) ledBlinkRed);
  addBitlashFunction("led.blinkgreen", (bitlash_function) ledBlinkGreen);
  addBitlashFunction("led.blinkblue", (bitlash_function) ledBlinkBlue);
  addBitlashFunction("led.blinkcyan", (bitlash_function) ledBlinkCyan);
  addBitlashFunction("led.blinkmagenta", (bitlash_function) ledBlinkMagenta);
  addBitlashFunction("led.blinkyellow", (bitlash_function) ledBlinkYellow);
  addBitlashFunction("led.blinkwhite", (bitlash_function) ledBlinkWhite);
  addBitlashFunction("led.redvalue", (bitlash_function) ledSetRedValue);
  addBitlashFunction("led.greenvalue", (bitlash_function) ledSetGreenValue);
  addBitlashFunction("led.bluevalue", (bitlash_function) ledSetBlueValue);
  addBitlashFunction("led.blinkwhite", (bitlash_function) ledBlinkWhite);
  addBitlashFunction("led.report", (bitlash_function) ledReport);
  
  addBitlashFunction("mesh.config", (bitlash_function) meshConfig);
  addBitlashFunction("mesh.key", (bitlash_function) meshSetKey);
  addBitlashFunction("mesh.remoterun", (bitlash_function) meshRemoteRun);
  addBitlashFunction("mesh.broadcastrun", (bitlash_function) meshBroadcastRun);
  addBitlashFunction("mesh.publish", (bitlash_function) meshPublish);
  addBitlashFunction("mesh.subscribe", (bitlash_function) meshSubscribe);
  
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
  //return Scout.getBatteryPercentage();
}

numvar getBatteryVoltage(void) {
  // TODO: broken, crashes board
  //return Scout.getBatteryVoltage();
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
numvar ledEnable(void) {
  RgbLed.enable();
}

numvar ledDisable(void) {
  RgbLed.disable();
}

numvar ledIsEnabled(void) {
  RgbLed.isEnabled();
}

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

numvar ledBlinkRed(void) {
  // TODO
  /*
  if (getarg(0) == 0) {
    RgbLed.blinkRed();
  } else {
    RgbLed.blinkRed(getarg(1));
  }
  */ 
}

numvar ledBlinkGreen(void) {
  RgbLed.blinkGreen(getarg(1));
}

numvar ledBlinkBlue(void) {
  RgbLed.blinkBlue(getarg(1));
}

numvar ledBlinkCyan(void) {
  RgbLed.blinkCyan(getarg(1));
}

numvar ledBlinkMagenta(void) {
  RgbLed.blinkMagenta(getarg(1));
}

numvar ledBlinkYellow(void) {
  RgbLed.blinkYellow(getarg(1));
}

numvar ledBlinkWhite(void) {
  RgbLed.blinkWhite(getarg(1));
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
  Pinoccio.meshSetSecurityKey((uint8_t*) getstringarg(1));
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
numvar ioGetPin(void) {
  // TODO: return JSON formatted status of a single pin
  return true;
}

numvar ioSetPin(void) {
  // TODO: set a pin's value
  return true;
}

numvar ioThreshold(void) {
  // TODO: create a threshold function with the following format:
  // threshold(pin, value, fnToCallIfValueLessThan, fnToCallIfValueEqual, fnToCallIfValueGreaterThan)
  return true;
}

numvar ioReport(void) {
  // TODO: return JSON formmated report of all IO pins and their values
  return true;
}

