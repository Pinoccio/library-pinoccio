#include <Arduino.h>
#include <Pinoccio.h>
#include "utility/atmega128rfa1.h"

Pinoccio::Pinoccio() { }

Pinoccio::~Pinoccio() { }

void Pinoccio::setup() {
  Serial.begin(115200);
  SYS_Init();

  // TODO--get from EEPROM
  NWK_SetAddr(1);
  NWK_SetPanId(0x4567);
  PHY_SetChannel(0x1a);
  PHY_SetRxState(true);

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

uint32_t Pinoccio::getRandomNumber() {
  PHY_RandomReq();
  return random();
}

uintptr_t Pinoccio::getFreeMemory() {
  extern uintptr_t __heap_start;
  extern void *__brkval;
  uintptr_t v;
  return (uintptr_t) &v - (__brkval == 0 ? (uintptr_t) &__heap_start : (uintptr_t) __brkval);
}

void Pinoccio::goToSleep() {
  // TODO
  // - put radio to sleep
  // - set all GPIO pins to inputs
  // - turn off ADC
  // - turn off backpack power
  // - put MCU to sleep
}

void Pinoccio::meshSendMessage(MeshRequest request) {

  // request.dstAddr = destinationAddr;
  // request.dstEndpoint = 1;
  // request.srcEndpoint = 1;
  // request.options = options;
  // request.data = message;
  // request.size = length;
  //TODO request.confirm = meshSendMessageConfirm;

  NWK_DataReq(&(request.data));
}

void Pinoccio::meshListenForMessagesAt(uint8_t endpoint, bool (*handler)(NWK_DataInd_t *ind)) {
  NWK_OpenEndpoint(endpoint, handler);
}

void Pinoccio::meshReceiveMessage(NWK_DataInd_t *ind) {
  Serial.println("received mesh message");
}

static void meshSendMessageConfirm(NWK_DataReq_t *req) {
 if (NWK_SUCCESS_STATUS == req->status)
   Serial.println("Message successfully sent");
 else {
   Serial.print("Error sending message: ");
   Serial.println(req->status, HEX);
 }
}

static bool meshReceiveMessage(NWK_DataInd_t *ind) {
  Pinoccio::meshReceiveMessage(ind);
}

// TODO
/*

bool Pinoccio::publish(char* topic, char* payload, int size) {

}

bool Pinoccio::subscribe(char* topic, bool (*handler)(NWK_DataInd_t *msg)) {

}
*/