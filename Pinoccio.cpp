#include <Arduino.h>
#include "Pinoccio.h"

PinoccioClass Pinoccio;

PinoccioClass::PinoccioClass() { 
	pinMode(CHG_STATUS, INPUT);
	digitalWrite(CHG_STATUS, HIGH);
	pinMode(BATT_CHECK, OUTPUT);
	digitalWrite(BATT_CHECK, HIGH);
}

PinoccioClass::~PinoccioClass() { } 

void PinoccioClass::alive() {
  Serial.println("Hello, I'm alive");
}

void PinoccioClass::init() {
  SYS_Init();
  HAL_MeasureAdcOffset();
}

void PinoccioClass::taskHandler() {
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