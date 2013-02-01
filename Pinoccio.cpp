#include <Arduino.h>
#include <Pinoccio.h>
#include "atmega128rfa1.h"

PinoccioClass Pinoccio;

PinoccioClass::PinoccioClass() {
  pinMode(CHG_STATUS, INPUT);
  pinMode(BATT_CHECK, OUTPUT);
  digitalWrite(BATT_CHECK, LOW);
  // TODO pinMode(VCC_ENABLE, OUTPUT);
  // digitalWrite(VCC_ENABLE, HIGH);
}

PinoccioClass::~PinoccioClass() { }

void PinoccioClass::alive() {
  Serial.println("Hello, I'm alive");
}

void PinoccioClass::init() {
  Serial.begin(115200);
  SYS_Init();
  // TODO PHY_TX_PWR_REG(TX_PWR_3_2DBM);
  HAL_MeasureAdcOffset();
}

void PinoccioClass::loop() {
  SYS_TaskHandler();
}

float PinoccioClass::getTemperature() {
  return HAL_MeasureTemperature();
}

bool PinoccioClass::isBatteryCharging() {
  return (digitalRead(CHG_STATUS) == 0);
}

float PinoccioClass::getBatteryVoltage() {
  digitalWrite(BATT_CHECK, HIGH);
  int read = analogRead(A7);
  digitalWrite(BATT_CHECK, LOW);
  return (read);
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