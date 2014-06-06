// This is based on and uses https://github.com/adafruit/Adafruit_nRF8001 just drop that in your libraries and this should build
/* wiring pin mapping is:
nRF8001 Scout
======= =====
SCK     SCK
MISO    MISO
MOSI    MOSI
REQ     SS
RDY     D4
ACT
RST     D8
3Vo
GND     GND
VIN     VUSB (also works powered via 3V3 when not plugged into USB)
*/

/*********************************************************************
This is an example for our nRF8001 Bluetooth Low Energy Breakout

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1697

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Kevin Townsend/KTOWN  for Adafruit Industries.
MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

// This version uses the internal data queing so you can treat it like Serial (kinda)!

#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>

#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#include "util/PrintToString.h"

// Connect CLK/MISO/MOSI to hardware SPI
// e.g. On UNO & compatible: CLK = 13, MISO = 12, MOSI = 11
#define ADAFRUITBLE_REQ SS
#define ADAFRUITBLE_RDY 4     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 8

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);
/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{ 
  Scout.setup("bluetooth", "custom", -1);
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(500);

  BTLEserial.begin();
}

/**************************************************************************/
/*!
    Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

StringBuffer bleOutput;
void loop()
{
  Scout.loop();
  // Tell the nRF8001 to do whatever it should be working on.
  BTLEserial.pollACI();

  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
        Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    laststatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {
    // Lets see if there's any data for us!
    StringBuffer bin(20);
    bin = "";
    while (BTLEserial.available()) {
      bin += (char)BTLEserial.read();
    }
    if(bin.length())
    {
      Serial.println(bin.c_str());
      setOutputHandler(&printToString<&bleOutput>);
      doCommand(const_cast<char *>(bin.c_str()));
      resetOutputHandler();

      Serial.println(bleOutput.c_str());
      BTLEserial.write((uint8_t*)bleOutput.c_str(), bleOutput.length());
      bleOutput = "";
      
    }

  }
}
