#include <Arduino.h>
#include <Pinoccio.h>
#include <bitlash.h>

#if defined(__AVR_ATmega128RFA1__)
#include "atmega128rfa1.h"
#elif defined(__AVR_ATmega256RFR2__)
#include "atmega256rfr2.h"
#endif

PinoccioClass Pinoccio;

PinoccioClass::PinoccioClass() { }

PinoccioClass::~PinoccioClass() { }

void PinoccioClass::setup() {
  Serial.begin(115200);
  SYS_Init();

  // TODO: PHY_TX_PWR_REG(TX_PWR_3_2DBM);
  HAL_MeasureAdcOffset();

  // initial seeding of RNG
  getRandomNumber();
  
  //initBitlash(115200);
}

void PinoccioClass::loop() {
  SYS_TaskHandler();
  //runBitlash();
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

void PinoccioClass::meshSetRadio(uint16_t addr, uint16_t panId, uint8_t channel) {
  // TODO--get from EEPROM -- last 32 bytes
  NWK_SetAddr(addr);
  NWK_SetPanId(panId);
  PHY_SetChannel(channel);
  PHY_SetRxState(true);
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