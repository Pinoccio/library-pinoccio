#include <Arduino.h>
#include <Pinoccio.h>

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
  
  initBitlash(115200);
}

void PinoccioClass::loop() {
  SYS_TaskHandler();
  runBitlash();
}

void PinoccioClass::goToSleep(uint32_t sleepForMs) {
  // TODO
  // - put radio to sleep
  // - set all GPIO pins to inputs
  // - turn off ADC
  // - turn off backpack power
  // - put MCU to sleep
}

float PinoccioClass::getTemperature() {
  return HAL_MeasureTemperature();
}

uint32_t PinoccioClass::getRandomNumber() {
  PHY_RandomReq();
  return random();
}

uintptr_t PinoccioClass::getFreeMemory() {
  extern uintptr_t __heap_start;
  extern void *__brkval;
  uintptr_t v;
  return (uintptr_t) &v - (__brkval == 0 ? (uintptr_t) &__heap_start : (uintptr_t) __brkval);
}

void PinoccioClass::meshSetRadio(uint16_t addr, uint16_t panId, uint8_t channel) {
  // TODO--get from EEPROM
  NWK_SetAddr(addr);
  NWK_SetPanId(panId);
  PHY_SetChannel(channel);
  PHY_SetRxState(true);
}
  
void PinoccioClass::meshSetSecurityKey(uint8_t *key) {
  // TODO:
  // NWK_SetSecurityKey(key);
}

void PinoccioClass::meshSendMessage(MeshRequest request) {
  NWK_DataReq(request.getRequest());
}

void PinoccioClass::meshListen(uint8_t endpoint, bool (*handler)(NWK_DataInd_t *ind)) {
  NWK_OpenEndpoint(endpoint, handler);
}