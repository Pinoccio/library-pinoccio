#include <Arduino.h>
#include <Pinoccio.h>
#include <bitlash.h>

#if defined(__AVR_ATmega128RFA1__)
#include "atmega128rfa1.h"
#elif defined(__AVR_ATmega256RFR2__)
#include "atmega256rfr2.h"
#endif

PinoccioClass Pinoccio;

PinoccioClass::PinoccioClass() { 
  shellEnabled = true;
}

PinoccioClass::~PinoccioClass() { }

void PinoccioClass::disableShell() {
  shellEnabled = false;
}

void PinoccioClass::setup() {
  if (shellEnabled) {
    initBitlash(115200);
  }
  
  SYS_Init();

  // TODO: PHY_TX_PWR_REG(TX_PWR_3_2DBM);
  HAL_MeasureAdcOffset();

  // initial seeding of RNG
  getRandomNumber();
}

void PinoccioClass::loop() {
  SYS_TaskHandler();
  
  if (shellEnabled) {
    runBitlash();
  }
}

void PinoccioClass::goToSleep(uint32_t sleepForMs) {
  // TODO
  // - put radio to sleep
  // - set all GPIO pins to inputs
  // - turn off ADC
  // - turn off backpack power
  // - put MCU to sleep
}

int8_t PinoccioClass::getTemperature() {
  return HAL_MeasureTemperature();
}

uint32_t PinoccioClass::getRandomNumber() {
  PHY_RandomReq();
  return random();
}

void PinoccioClass::meshSetRadio(uint16_t theAddress, uint16_t thePanId, uint8_t theChannel) {
  // TODO--get from EEPROM -- last 32 bytes
  NWK_SetAddr(theAddress);
  address = theAddress;
  NWK_SetPanId(thePanId);
  panId = thePanId;
  PHY_SetChannel(theChannel);
  channel = theChannel;
  PHY_SetRxState(true);
  meshSetPower(0);
}
 
 
void PinoccioClass::meshSetPower(uint8_t theTxPower) {
  /* Page 116 of the 256RFR2 datasheet 
    0   3.5 dBm
    1   3.3 dBm
    2   2.8 dBm
    3   2.3 dBm
    4   1.8 dBm
    5   1.2 dBm
    6   0.5 dBm
    7  -0.5 dBm
    8  -1.5 dBm
    9  -2.5 dBm
    10 -3.5 dBm
    11 -4.5 dBm
    12 -6.5 dBm
    13 -8.5 dBm
    14 -11.5 dBm
    15 -16.5 dBm
  */
  PHY_SetTxPower(theTxPower);
  txPower = theTxPower;
} 

void PinoccioClass::meshSetSecurityKey(const char *key) {
  NWK_SetSecurityKey((uint8_t *)key);
}

void PinoccioClass::meshSendMessage(MeshRequest request) {
  NWK_DataReq_t* req = request.getRequest();
  Serial.print("sending message to: ");
  Serial.print(req->dstAddr);
  Serial.print(":");
  Serial.println(req->dstEndpoint);
  NWK_DataReq(request.getRequest());
}

void PinoccioClass::meshListen(uint8_t endpoint, bool (*handler)(NWK_DataInd_t *ind)) {
  NWK_OpenEndpoint(endpoint, handler);
}

void PinoccioClass::meshJoinGroup(uint16_t groupAddress) {
  if (!NWK_GroupIsMember(groupAddress)) {
      NWK_GroupAdd(groupAddress);
    }
}

void PinoccioClass::meshLeaveGroup(uint16_t groupAddress) {
  if (NWK_GroupIsMember(groupAddress)) {
      NWK_GroupRemove(groupAddress);
    }
}

uint16_t PinoccioClass::getAddress() {
  return address;
}

uint16_t PinoccioClass::getPanId() {
  return panId;
}

uint8_t PinoccioClass::getChannel() {
  return channel;
}

uint8_t PinoccioClass::getTxPower() {
  return txPower;
}