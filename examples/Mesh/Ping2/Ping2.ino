#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>

byte pingCounter = 0;
int meshAddress = 1;

void setup() {
  Scout.setup();
  Scout.meshSetRadio(meshAddress);
  Scout.meshListen(1, receiveMessage);
}

void loop() {
  Scout.loop();
  sendMessage();
  
  RgbLed.blinkCyan();
  delay(500);
}


static void sendMessage(void) {
  static MeshRequest request = MeshRequest();
  
  if (meshAddress == 1) {
    request.setDstAddress(2);
  } else {
    request.setDstAddress(1);
  }
  request.setPayload(&pingCounter, sizeof(pingCounter));
  request.setConfirmCallback(sendMessageConfirm);

  Scout.meshSendMessage(request);

  Serial.print("Sent Message with data: ");
  Serial.println(pingCounter);

  pingCounter++;
}

static void sendMessageConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
  } else {
    Serial.print("error: ");
    Serial.println(req->status);
  }
}

static bool receiveMessage(NWK_DataInd_t *ind) {
  Serial.print("Received message - ");
  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);

  Serial.print("  ");

  Serial.print("rssi: ");
  Serial.print(ind->rssi, DEC);
  Serial.print("  ");

  Serial.print("data: ");
  for (int i=0; i<ind->size; i++) {
    Serial.print(ind->data[i], DEC);
  }
  Serial.println("");
  return true;
}
