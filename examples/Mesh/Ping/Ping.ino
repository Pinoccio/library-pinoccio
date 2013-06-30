#include "config.h"
#include <Pinoccio.h>

static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appPingCounter = 0;
static bool receivedPacket = false;

static void appSendDataConf(NWK_DataReq_t *req) {
  appDataReqBusy = false;
  appSendData();
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
  if (APP_ADDR == 0) {
    appSendData();
    (void)timer;
  }
  if (APP_ADDR == 1) {
    Serial.println("Waiting....");
  }
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
    Serial.write(ind->data[i]);
  }
  Serial.println("");
  return true;
}

void setup() {
  Pinoccio.init();
  
  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);
  NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

  appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
  appTimer.mode = SYS_TIMER_PERIODIC_MODE;
  appTimer.handler = appTimerHandler;
  SYS_TimerStart(&appTimer);

  RgbLed.cyan();
  appPingCounter = 0;
}

void loop() {
  Pinoccio.loop();
}
