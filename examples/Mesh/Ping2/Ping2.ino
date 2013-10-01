#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>

int meshAddress = 1; // Set to 1 for the sender, set to 2 for the receiver

byte pingCounter = 0;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;

void setup() {
  Scout.setup();
  Scout.meshSetRadio(meshAddress);
  Scout.meshSetSecurityKey("TestSecurityKey1");
  
  if (meshAddress == 1) {
    appTimer.interval = 30000;
    appTimer.mode = SYS_TIMER_PERIODIC_MODE;
    appTimer.handler = sendMessage;
    SYS_TimerStart(&appTimer);
  } else {
    Serial.println("Waiting for ping packets:");
    Scout.meshListen(1, receiveMessage);  
  }
}

void loop() {
  Scout.loop();
}

static void sendMessage(SYS_Timer_t *timer) {  
  appDataReq.dstAddr = 2;
  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = sendMessageConfirm;
  NWK_DataReq(&appDataReq);
  RgbLed.blinkCyan(200);

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
    Serial.println(req->status, HEX);
  }
}

static bool receiveMessage(NWK_DataInd_t *ind) {
  RgbLed.blinkGreen(200);
  
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