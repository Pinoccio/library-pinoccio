#include <Arduino.h>
#include <Wire.h>
#include <Scout.h>

int meshAddress = 2; // Set to 1 for the sender, set to 2 for the receiver
const uint16_t groupId = 0xBEEF;

static byte pingCounter = 0;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;

void setup() {
  //Scout.disableShell();
  addBitlashFunction("mesh.config", (bitlash_function) meshConfig);
  addBitlashFunction("mesh.ping", (bitlash_function) meshPing);
  addBitlashFunction("mesh.joingroup", (bitlash_function) meshJoinGroup);
  addBitlashFunction("mesh.leavegroup", (bitlash_function) meshLeaveGroup);
  addBitlashFunction("mesh.pinggroup", (bitlash_function) meshPingGroup);
  addBitlashFunction("mesh.report", (bitlash_function) meshReport);
  
  Scout.setup();
  Scout.meshJoinGroup(groupId);
  
  Scout.meshSetRadio(meshAddress);
  Scout.meshSetSecurityKey("TestSecurityKey1");
  
  Scout.meshListen(1, receiveMessage);  
}

void loop() {
  Scout.loop();
}

numvar meshConfig(void) {
  uint16_t panId = 0x4567;
  uint8_t channel = 0x1a;
  if (getarg(0) == 2) {
    panId = getarg(2);
    channel = getarg(3);
  }
  Pinoccio.meshSetRadio(getarg(1), panId, channel);
}

numvar meshPing(void) {
  pingScout(getarg(1));
}

numvar meshJoinGroup(void) {
  Scout.meshJoinGroup(groupId);
}

numvar meshLeaveGroup(void) {
  Scout.meshLeaveGroup(groupId);
}

numvar meshPingGroup(void) {
  pingGroup(groupId);
}

numvar meshReport(void) {
  Serial.println("-- Mesh Radio Settings --");
  Serial.print("      Address: ");
  Serial.println(Scout.getAddress());
  Serial.print("       Pan ID: 0x");
  Serial.println(Scout.getPanId(), HEX);
  Serial.print("      Channel: ");
  Serial.println(Scout.getChannel());
  Serial.print("     Tx Power: ");
  // gotta read these from program memory (for SRAM savings)
  char c;
  const char *dbString = Scout.getTxPowerDb();
  while((c = pgm_read_byte(dbString++))) {
     Serial.write(c);
  }
  Serial.println();
// TODO
//  Serial.print("   -   Asleep: ");
//  Serial.println();
//  Serial.print("   - In group: ");
//  Serial.println();
//  Serial.print("   -  Routing: ");
//  Serial.println();
  
}

static void pingScout(int address) {  
  appDataReq.dstAddr = address;

  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = pingConfirm;
  NWK_DataReq(&appDataReq);
  //RgbLed.blinkCyan(200);

  Serial.print("PING ");
  Serial.print(address);
  Serial.print(": "); 

  pingCounter++;
}

static void pingGroup(int address) {  
  appDataReq.dstAddr = address;

  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_MULTICAST|NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = pingConfirm;
  NWK_DataReq(&appDataReq);

  Serial.print("PING ");
  Serial.print(address);
  Serial.print(": "); 

  pingCounter++;
}

static void pingConfirm(NWK_DataReq_t *req) {
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.print("1 byte from ");
    Serial.print(req->dstAddr);
    Serial.print(" RSSI=-");
    Serial.println(req->control);
  } else {
    Serial.print("Error: ");
    switch (req->status) {
      case NWK_OUT_OF_MEMORY_STATUS:
        Serial.print("Out of memory: ");
        break;
      case NWK_NO_ACK_STATUS:
      case NWK_PHY_NO_ACK_STATUS:
        Serial.print("No acknowledgement received: ");
        break;
      case NWK_NO_ROUTE_STATUS:
        Serial.print("No route to destination: ");
        break;
      case NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS:
        Serial.print("Physical channel access failure: ");
        break;
      default:
        Serial.print("unknown failure: ");
    }
    Serial.print("(");
    Serial.print(req->status, HEX);
    Serial.println(")");
  }
}

static bool receiveMessage(NWK_DataInd_t *ind) {  
  Serial.print("Received message - ");
  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);

  Serial.print("  ");

  Serial.print("rssi: ");
  Serial.print(abs(ind->rssi), DEC);
  Serial.print("  ");

  Serial.print("data: ");
  for (int i=0; i<ind->size; i++) {
    Serial.print(ind->data[i], DEC);
  }
  Serial.println("");
  NWK_SetAckControl(abs(ind->rssi));
  return true;
}

