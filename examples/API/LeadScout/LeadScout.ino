#include "config.h"
#include <Pinoccio.h>

// TODO 
WIFI_PROFILE profile = {
                /* SSID */ "Radio Free Epnk",
 /* WPA/WPA2 passphrase */ "tenor78!heavyweight",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };
          
//IPAddress server(66,175,218,211);
IPAddress server(85,119,83,194);

static NWK_DataReq_t nwkDataReq;
             
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received MQTT packet from topic : ");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");
  Serial.println("Sending message over mesh network");
  sendMessage(payload, length);
}  

static void appDataConf(NWK_DataReq_t *req) {
 if (NWK_SUCCESS_STATUS == req->status)
   Serial.println("Message successfully sent");
 else {
   Serial.print("Error sending message: ");
   Serial.println(req->status, HEX);
 }
}
           
static void sendMessage(byte* message, unsigned int length) {
   nwkDataReq.dstAddr = 3;
   nwkDataReq.dstEndpoint = 1;
   nwkDataReq.srcEndpoint = 1;
   nwkDataReq.options = 0;
   nwkDataReq.data = message;
   nwkDataReq.size = length;
   nwkDataReq.confirm = appDataConf;
   NWK_DataReq(&nwkDataReq);
}

PinoccioWifiClient wifiClient;
mqttClient mqtt(server, 1883, callback, wifiClient);

static SYS_Timer_t appTimer;

void setup() {
  Pinoccio.init();
  NWK_SetAddr(APP_MESH_ADDR);
  NWK_SetPanId(APP_MESH_PANID);
  PHY_SetChannel(APP_MESH_CHANNEL);
  PHY_SetRxState(true);
  
  Serial.print("Starting wireless...");
  Wifi.begin(&profile);
  Serial.println("Done");
  
  appTimer.interval = 5000;
  appTimer.mode = SYS_TIMER_PERIODIC_MODE;
  appTimer.handler = appTimerHandler;
  SYS_TimerStart(&appTimer);
  
  Serial.println("Connecting to MQTT server...");
  if (mqtt.connect("pinoccio", "username", "password")) {
    Serial.println("Done");
    mqtt.subscribe("erictj/3pi-control");
  }
}

void loop() {
  Pinoccio.loop();
  mqtt.loop();
}

static void appTimerHandler(SYS_Timer_t *timer) {
  mqtt.publish("erictj/3pi-telemetry", "Still alive");
  RgbLed.blinkCyan(200);
}