#include <Arduino.h>
#include <Pinoccio.h>
#include "utility/atmega128rfa1.h"

Pinoccio::Pinoccio() { }

Pinoccio::~Pinoccio() { }

void Pinoccio::setup() {
  Serial.begin(115200);
  SYS_Init();
  // TODO: PHY_TX_PWR_REG(TX_PWR_3_2DBM);
  HAL_MeasureAdcOffset();
  
  // initial seeding of RNG
  getRandomNumber();
}

void Pinoccio::loop() {
  SYS_TaskHandler();
}

float Pinoccio::getTemperature() {
  return HAL_MeasureTemperature();
}

uintptr_t Pinoccio::getFreeMemory() {
  extern uintptr_t __heap_start;
  extern void *__brkval; 
  uintptr_t v; 
  return (uintptr_t) &v - (__brkval == 0 ? (uintptr_t) &__heap_start : (uintptr_t) __brkval); 
}

uint32_t Pinoccio::getRandomNumber() {
  PHY_RandomReq();
  return random();
}

// TODO
/*
void Pinoccio::initMesh() {
  NWK_SetAddr(APP_MESH_ADDR);
  NWK_SetPanId(APP_MESH_PANID);
  PHY_SetChannel(APP_MESH_CHANNEL);
  PHY_SetRxState(true);
}
*/
/*

bool Pinoccio::publish(char* topic, char* payload, int size) {

}

bool Pinoccio::subscribe(char* topic, bool (*handler)(NWK_DataInd_t *msg)) {

}

bool sendMessage(NWK_DataReq_t *msg) {
  NWK_DataReq(msg);
}

bool listenForMessage(uint8_t id, bool (*handler)(NWK_DataInd_t *msg)) {
  NWK_OpenEndpoint(id, handler);
}
*/