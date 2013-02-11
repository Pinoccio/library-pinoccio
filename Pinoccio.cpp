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

bool PinoccioClass::isBatteryPresent() {
  return (triStateBatteryCheck() == 1);
}

bool PinoccioClass::isBatteryCharging() {
  //return (triStateBatteryCheck() == 0);
  return (digitalRead(CHG_STATUS) == LOW);
}

uint8_t PinoccioClass::triStateBatteryCheck() {
  // TODO, get this working:  http://forums.parallax.com/showthread.php/141525-Tri-State-Logic-and-a-propeller-input?p=1113946&viewfull=1#post1113946
  // returns 0 for charging, 1 for no battery, and 3 for charged
  uint8_t state = 0;

  // The DDxn bit in the DDRx Register selects the direction of this pin. 1==output, 0==input
  DDRD = (1<<DDB7);

  // If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is activated.
  // To switch the pull-up resistor off, PORTxn has to be written logic zero or the pin has to be configured as an output pin.
  // If PORTxn is written logic one when the pin is configured as an output pin, output is HIGH, else LOW
  PORTD = (0<<PD7);
  DDRD = (0<<DDB7);
  PORTD = (1<<PD7);
  asm volatile("nop");
  state = PIND;
  DDRD = (1<<DDB7);
  DDRD = (0<<DDB7);
  state <<= 1;
  state |= PIND;
  
  return state;
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

void PinoccioClass::enableShieldVcc() {
  digitalWrite(VCC_ENABLE, HIGH);
}

void PinoccioClass::disableShieldVcc() {
  digitalWrite(VCC_ENABLE, LOW);
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