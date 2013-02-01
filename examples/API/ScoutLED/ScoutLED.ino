#include "config.h"
#include <Pinoccio.h>

void setup() {
  Pinoccio.init();
  RgbLed.blinkRed(200);
  RgbLed.blinkGreen(200);
  RgbLed.blinkBlue(200);
  Serial.println("Scout ready for duty");
  
  NWK_SetAddr(APP_MESH_ADDR);
  NWK_SetPanId(APP_MESH_PANID);
  PHY_SetChannel(APP_MESH_CHANNEL);
  PHY_SetRxState(true);
  
  NWK_OpenEndpoint(1, appDataInd);
  //Pinoccio.subscribe("erictj/led");
}

void loop() {
  Pinoccio.loop();
}

static bool appDataInd(NWK_DataInd_t *ind) {
  // process the frame
  RgbLed.blinkCyan(200);
  Serial.println("srcAddr: " + ind->srcAddr);
  Serial.println("dstEndpoint: " + ind->dstEndpoint);
  Serial.println("srcEndpoint: " + ind->srcEndpoint);
  Serial.println("options: " + ind->options);
  Serial.print("data: ");
  Serial.write(ind->data, ind->size);
  Serial.println("");
  Serial.println("lqi: " + ind->lqi);
  Serial.println("rssi: " + ind->rssi);
  return true;
}