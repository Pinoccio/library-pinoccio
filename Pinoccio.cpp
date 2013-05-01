#include <Arduino.h>
#include <Pinoccio.h>
#include "atmega128rfa1.h"

PinoccioClass Pinoccio;

PinoccioClass::PinoccioClass() {
  RgbLed.turnOff();
  pinMode(CHG_STATUS, INPUT);
  digitalWrite(BATT_CHECK, LOW);
  pinMode(BATT_CHECK, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  pinMode(VCC_ENABLE, OUTPUT);
}

PinoccioClass::~PinoccioClass() { }

void PinoccioClass::init() {
  Serial.begin(115200);
  SYS_Init();
  // TODO: PHY_TX_PWR_REG(TX_PWR_3_2DBM);
  HAL_MeasureAdcOffset();
}

void PinoccioClass::loop() {
  SYS_TaskHandler();
}

float PinoccioClass::getTemperature() {
  return HAL_MeasureTemperature();
}

bool PinoccioClass::isBatteryCharging() {
  //return (triStateBatteryCheck() == 0);
  return (digitalRead(CHG_STATUS) == LOW);
}

float PinoccioClass::getBatteryVoltage() {
  pinMode(A7, INPUT);
  digitalWrite(A7, LOW);
  digitalWrite(BATT_CHECK, HIGH);
  int read = analogRead(A7);
  digitalWrite(BATT_CHECK, LOW);
  digitalWrite(A7, HIGH);
  return read;
}

void PinoccioClass::enableBackpackVcc() {
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioClass::disableBackpackVcc() {
  digitalWrite(VCC_ENABLE, LOW);
}

uintptr_t PinoccioClass::getFreeMemory() {
  extern uintptr_t __heap_start;
  extern void *__brkval; 
  uintptr_t v; 
  return (uintptr_t) &v - (__brkval == 0 ? (uintptr_t) &__heap_start : (uintptr_t) __brkval); 
}

void PinoccioClass::setRandomNumber(uint16_t number) {
  randomNumber = number;
}

// TODO
/*
void PinoccioClass::initMesh() {
  NWK_SetAddr(APP_MESH_ADDR);
  NWK_SetPanId(APP_MESH_PANID);
  PHY_SetChannel(APP_MESH_CHANNEL);
  PHY_SetRxState(true);
}
*/
/*

bool PinoccioClass::publish(char* topic, char* payload, int size) {

}

bool PinoccioClass::subscribe(char* topic, bool (*handler)(NWK_DataInd_t *msg)) {

}

bool sendMessage(NWK_DataReq_t *msg) {
  NWK_DataReq(msg);
}

bool listenForMessage(uint8_t id, bool (*handler)(NWK_DataInd_t *msg)) {
  NWK_OpenEndpoint(id, handler);
}
*/

// TODO
/*
#ifdef __cplusplus
extern "C"{
#endif

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
PHY_RandomConf(rnd) {
  Pinoccio.setRandomNumber(rnd);
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif
*/