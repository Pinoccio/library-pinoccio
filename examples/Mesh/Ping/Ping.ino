#include "config.h"
#include <Scout.h>

static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appPingCounter = 0;
static bool receivedPacket = false;

static void appSendDataConf(NWK_DataReq_t *req) {
  appDataReqBusy = false;
  if (NWK_SUCCESS_STATUS == req->status) {
    Serial.println("- received response");
  } else {
     Serial.println("- error, no response received"); 
  }
  //appSendData();
  (void)req;
}

static void appSendData(void) {
  if (appDataReqBusy) {
    return;
  }

  appDataReq.dstAddr = 1;
  appDataReq.dstEndpoint = APP_ENDPOINT;
  appDataReq.srcEndpoint = APP_ENDPOINT;
  appDataReq.options = NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &appPingCounter;
  appDataReq.size = sizeof(appPingCounter);
  appDataReq.confirm = appSendDataConf;
  NWK_DataReq(&appDataReq);

  appPingCounter++;
  appDataReqBusy = true;
}

static void appTimerHandler(SYS_Timer_t *timer) {
  appSendData();
  Serial.println("Sent data");
  RgbLed.blinkCyan(200);
}

static bool appDataInd(NWK_DataInd_t *ind) {
  receivedPacket = true;
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

void setup() {
  SYS_Init();
  //Scout.setup();
  Serial.begin(115200);
  
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);
  NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

  appPingCounter = 0;
  
  if (APP_ADDR == 1) {
    Serial.println("Waiting for ping packets:");
  } else {
    appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
    appTimer.mode = SYS_TIMER_PERIODIC_MODE;
    appTimer.handler = appTimerHandler;
    SYS_TimerStart(&appTimer);
  }
}

void loop() {
  SYS_TaskHandler();
}