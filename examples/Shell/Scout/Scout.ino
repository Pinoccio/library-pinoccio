#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <PBBP.h>
#include <utility/WiFiBackpack.h>
extern "C" {
#include <j0g.h>
#include "utility/sysTimer.h"
}

const uint16_t groupId = 0x1234;

WiFiBackpack wifi = WiFiBackpack();

// use this if your lead scout doesn't have the backpack bus supporting firmware
bool forceLeadScout = false;
bool forceScoutVersion = true;

// this stuff should prob all be in the Scout class or somesuch but putting it here to get started
static bool fieldCommands(NWK_DataInd_t *ind);
int leadAnswerID = 0;
static bool leadAnswers(NWK_DataInd_t *ind);
static bool leadAnnouncements(NWK_DataInd_t *ind);
void leadAnnouncementSend(int chan, int from, char *message);

int whoami;

bool isLeadScout;

void leadHQ(void);
void leadSignal(char *json);
void leadIncoming(char *packet, unsigned short *index);
NWK_DataReq_t fieldAnnounceReq;
void fieldAnnounce(char *line); // called by bitlashFilter
void bitlashFilter(byte b);
char *bitlashOutput; // used by bitlashBuffer
void bitlashBuffer(byte b);

// Example code to dump backpack EEPROM contents:
void print_hex(const uint8_t *buf, uint8_t len) {
    while (len--) {
        if (*buf < 0x10) Serial.print("0");
        Serial.print(*buf++, HEX);
    }
}

void dump_backpacks() {
  Serial.print("Found ");
  Serial.print(Scout.bp.num_slaves);
  Serial.println(" slaves");

  for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
    print_hex(Scout.bp.slave_ids[i], sizeof(Scout.bp.slave_ids[0]));
    Serial.println();
    uint8_t buf[64];
    Scout.bp.readEeprom(i + 1, 0, buf, sizeof(buf));
    Serial.print("EEPROM: ");
    print_hex(buf, sizeof(buf));
    Serial.println();
  }
}
// Geotrust Global CA (used by google.com)
unsigned char cert[] = {0x30, 0x82, 0x03, 0x54, 0x30, 0x82, 0x02, 0x3c, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x03, 0x02, 0x34, 0x56, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x30, 0x32, 0x30, 0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x32, 0x32, 0x30, 0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43, 0x41, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xda, 0xcc, 0x18, 0x63, 0x30, 0xfd, 0xf4, 0x17, 0x23, 0x1a, 0x56, 0x7e, 0x5b, 0xdf, 0x3c, 0x6c, 0x38, 0xe4, 0x71, 0xb7, 0x78, 0x91, 0xd4, 0xbc, 0xa1, 0xd8, 0x4c, 0xf8, 0xa8, 0x43, 0xb6, 0x03, 0xe9, 0x4d, 0x21, 0x07, 0x08, 0x88, 0xda, 0x58, 0x2f, 0x66, 0x39, 0x29, 0xbd, 0x05, 0x78, 0x8b, 0x9d, 0x38, 0xe8, 0x05, 0xb7, 0x6a, 0x7e, 0x71, 0xa4, 0xe6, 0xc4, 0x60, 0xa6, 0xb0, 0xef, 0x80, 0xe4, 0x89, 0x28, 0x0f, 0x9e, 0x25, 0xd6, 0xed, 0x83, 0xf3, 0xad, 0xa6, 0x91, 0xc7, 0x98, 0xc9, 0x42, 0x18, 0x35, 0x14, 0x9d, 0xad, 0x98, 0x46, 0x92, 0x2e, 0x4f, 0xca, 0xf1, 0x87, 0x43, 0xc1, 0x16, 0x95, 0x57, 0x2d, 0x50, 0xef, 0x89, 0x2d, 0x80, 0x7a, 0x57, 0xad, 0xf2, 0xee, 0x5f, 0x6b, 0xd2, 0x00, 0x8d, 0xb9, 0x14, 0xf8, 0x14, 0x15, 0x35, 0xd9, 0xc0, 0x46, 0xa3, 0x7b, 0x72, 0xc8, 0x91, 0xbf, 0xc9, 0x55, 0x2b, 0xcd, 0xd0, 0x97, 0x3e, 0x9c, 0x26, 0x64, 0xcc, 0xdf, 0xce, 0x83, 0x19, 0x71, 0xca, 0x4e, 0xe6, 0xd4, 0xd5, 0x7b, 0xa9, 0x19, 0xcd, 0x55, 0xde, 0xc8, 0xec, 0xd2, 0x5e, 0x38, 0x53, 0xe5, 0x5c, 0x4f, 0x8c, 0x2d, 0xfe, 0x50, 0x23, 0x36, 0xfc, 0x66, 0xe6, 0xcb, 0x8e, 0xa4, 0x39, 0x19, 0x00, 0xb7, 0x95, 0x02, 0x39, 0x91, 0x0b, 0x0e, 0xfe, 0x38, 0x2e, 0xd1, 0x1d, 0x05, 0x9a, 0xf6, 0x4d, 0x3e, 0x6f, 0x0f, 0x07, 0x1d, 0xaf, 0x2c, 0x1e, 0x8f, 0x60, 0x39, 0xe2, 0xfa, 0x36, 0x53, 0x13, 0x39, 0xd4, 0x5e, 0x26, 0x2b, 0xdb, 0x3d, 0xa8, 0x14, 0xbd, 0x32, 0xeb, 0x18, 0x03, 0x28, 0x52, 0x04, 0x71, 0xe5, 0xab, 0x33, 0x3d, 0xe1, 0x38, 0xbb, 0x07, 0x36, 0x84, 0x62, 0x9c, 0x79, 0xea, 0x16, 0x30, 0xf4, 0x5f, 0xc0, 0x2b, 0xe8, 0x71, 0x6b, 0xe4, 0xf9, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x53, 0x30, 0x51, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xc0, 0x7a, 0x98, 0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d, 0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xc0, 0x7a, 0x98, 0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d, 0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x35, 0xe3, 0x29, 0x6a, 0xe5, 0x2f, 0x5d, 0x54, 0x8e, 0x29, 0x50, 0x94, 0x9f, 0x99, 0x1a, 0x14, 0xe4, 0x8f, 0x78, 0x2a, 0x62, 0x94, 0xa2, 0x27, 0x67, 0x9e, 0xd0, 0xcf, 0x1a, 0x5e, 0x47, 0xe9, 0xc1, 0xb2, 0xa4, 0xcf, 0xdd, 0x41, 0x1a, 0x05, 0x4e, 0x9b, 0x4b, 0xee, 0x4a, 0x6f, 0x55, 0x52, 0xb3, 0x24, 0xa1, 0x37, 0x0a, 0xeb, 0x64, 0x76, 0x2a, 0x2e, 0x2c, 0xf3, 0xfd, 0x3b, 0x75, 0x90, 0xbf, 0xfa, 0x71, 0xd8, 0xc7, 0x3d, 0x37, 0xd2, 0xb5, 0x05, 0x95, 0x62, 0xb9, 0xa6, 0xde, 0x89, 0x3d, 0x36, 0x7b, 0x38, 0x77, 0x48, 0x97, 0xac, 0xa6, 0x20, 0x8f, 0x2e, 0xa6, 0xc9, 0x0c, 0xc2, 0xb2, 0x99, 0x45, 0x00, 0xc7, 0xce, 0x11, 0x51, 0x22, 0x22, 0xe0, 0xa5, 0xea, 0xb6, 0x15, 0x48, 0x09, 0x64, 0xea, 0x5e, 0x4f, 0x74, 0xf7, 0x05, 0x3e, 0xc7, 0x8a, 0x52, 0x0c, 0xdb, 0x15, 0xb4, 0xbd, 0x6d, 0x9b, 0xe5, 0xc6, 0xb1, 0x54, 0x68, 0xa9, 0xe3, 0x69, 0x90, 0xb6, 0x9a, 0xa5, 0x0f, 0xb8, 0xb9, 0x3f, 0x20, 0x7d, 0xae, 0x4a, 0xb5, 0xb8, 0x9c, 0xe4, 0x1d, 0xb6, 0xab, 0xe6, 0x94, 0xa5, 0xc1, 0xc7, 0x83, 0xad, 0xdb, 0xf5, 0x27, 0x87, 0x0e, 0x04, 0x6c, 0xd5, 0xff, 0xdd, 0xa0, 0x5d, 0xed, 0x87, 0x52, 0xb7, 0x2b, 0x15, 0x02, 0xae, 0x39, 0xa6, 0x6a, 0x74, 0xe9, 0xda, 0xc4, 0xe7, 0xbc, 0x4d, 0x34, 0x1e, 0xa9, 0x5c, 0x4d, 0x33, 0x5f, 0x92, 0x09, 0x2f, 0x88, 0x66, 0x5d, 0x77, 0x97, 0xc7, 0x1d, 0x76, 0x13, 0xa9, 0xd5, 0xe5, 0xf1, 0x16, 0x09, 0x11, 0x35, 0xd5, 0xac, 0xdb, 0x24, 0x71, 0x70, 0x2c, 0x98, 0x56, 0x0b, 0xd9, 0x17, 0xb4, 0xd1, 0xe3, 0x51, 0x2b, 0x5e, 0x75, 0xe8, 0xd5, 0xd0, 0xdc, 0x4f, 0x34, 0xed, 0xc2, 0x05, 0x66, 0x80, 0xa1, 0xcb, 0xe6, 0x33};

static void hqConnectHandler(uint8_t cid) {
  leadHQConnect(cid);
}

static void hqDisconnectHandler(uint8_t cid) {
  Serial.println("Disconnected from HQ");
}

void setup(void) {
  // set up event handlers
  Scout.digitalPinEventHandler = digitalPinEventHandler;
  Scout.analogPinEventHandler = analogPinEventHandler;
  Scout.batteryPercentageEventHandler = batteryPercentageEventHandler;
  Scout.batteryVoltageEventHandler = batteryVoltageEventHandler;
  Scout.batteryChargingEventHandler = batteryChargingEventHandler;
  Scout.batteryAlarmTriggeredEventHandler = batteryAlarmTriggeredEventHandler;
  Scout.temperatureEventHandler = temperatureEventHandler;

  Scout.setup();

  isLeadScout = forceLeadScout ? true : Scout.isLeadScout();

  if (isLeadScout) {
    Gainspan.connectEventHandler = hqConnectHandler;
    Gainspan.disconnectEventHandler = hqDisconnectHandler;
    Serial.print("Wi-Fi backpack connecting...");
    wifi.setup();
    wifi.init();
    Serial.println("Done");
    RgbLed.blinkGreen();
  }

  Scout.meshListen(1, receiveMessage);

  //dump_backpacks();

  whoami = Scout.getAddress();
  if (isLeadScout) {
    Scout.meshListen(3, leadAnswers);
    Scout.meshListen(4, leadAnnouncements);
    // join all the "channels" to listen for announcements
    for (int i = 1; i < 10; i++) Scout.meshJoinGroup(i);
    Scout.meshJoinGroup(0xbeef); // our internal reporting channel
    Serial.println("Lead Scout ready!");
  } else {
    Scout.meshListen(2, fieldCommands);
    Serial.println("Field Scout ready!");
  }

  setOutputHandler(&bitlashFilter);
}

void loop(void) {
  Scout.loop();
  if (isLeadScout) {
    wifi.loop();
    leadHQ();
  }
}

////////////////////
// lead scout stuff

void leadHQConnect(uint8_t cid) {
  char auth[256], token[33];

  if (wifi.client.connected()) {
    Pinoccio.getHQToken(token);
    token[32] = 0;
    sprintf(auth,"{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    leadSignal(auth);
  } else {
    Serial.println("server unvailable");
  }
}

// this is called on the main loop to try to (re)connect to the HQ
void leadHQ(void)
{
  static char *buffer = NULL;
  uint8_t block[128];
  char *nl;
  int rsize, len, i;
  unsigned short index[32]; // <10 keypairs in the incoming json

  // only continue if new data to read
  if(!wifi.client.available()) {
    return;
  }

  // check to initialize our read buffer
  if(!buffer) {
    buffer = (char*)malloc(1);
    *buffer = 0;
  }

  // get all waiting data and look for packets
  while(rsize = wifi.client.read(block, 256)){
    len = strlen(buffer);

    // process chunk of incoming data
    buffer = (char*)realloc(buffer, len+rsize+1);
    if(!buffer) return; // TODO, realloc error, need to restart?
    memcpy(buffer+len, block, rsize);
    buffer[len+rsize] = 0; // null terminate

    // look for a packet
    Serial.print("looking for packet in: ");
    Serial.println(buffer);
    nl = strchr(buffer, '\n');
    if(!nl) continue;

    // null terminate just the packet and process it
    *nl = 0;
    j0g(buffer, index, 32);
    if(*index) leadIncoming(buffer, index);
    else Serial.println("JSON parse failed");

    // advance buffer and resize, minimum is just the buffer end null
    nl++;
    len = strlen(nl);
    memmove(buffer, nl, len+1);
    buffer = (char*)realloc(buffer, len+1); // shrink
  }
}

// when we can't process a command for some internal reason
void leadCommandError(int from, int id, char *reason)
{
  char *err;
  err = (char*)malloc(strlen(reason)+128);
  sprintf(err,"{\"type\":\"reply\",\"from\":%d,\"id\":%d,\"err\":true,\"reply\":\"%s\"}\n",from,id,reason);
  leadSignal(err);
  free(err);
}

// necessities for tracking state when chunking up a large command into mesh requests
int leadCommandTo = 0;
char *leadCommandChunks;
int leadCommandChunksAt;
int leadCommandRetries;
NWK_DataReq_t leadCommandReq;
void leadCommandChunk(void);

// process a packet from HQ
void leadIncoming(char *packet, unsigned short *index)
{
  char *type, *command;
  int to, ret, len;
  unsigned long id;

  type = j0g_str("type", packet, index);
  Serial.println(type);

  if(strcmp(type, "online") == 0)
  {
    // TODO anything hard-coded to do once confirmed online?
  }

  if(strcmp(type, "command") == 0)
  {
    to = atoi(j0g_str("to", packet, index));
    id = strtoul(j0g_str("id", packet, index), NULL, 10);
    command = j0g_str("command", packet, index);
    if(strlen(j0g_str("to", packet, index)) == 0 || !id || !command)
    {
      Serial.println("invalid command, requires to, id, command");
      Serial.print("to: ");
      Serial.println(to);
      Serial.print("id: ");
      Serial.println(id);
      Serial.print("command: ");
      Serial.println(command);
      return;
    }

    // handle internal ones first
    if(to == whoami)
    {

      bitlashOutput = (char*)malloc(255);

      sprintf(bitlashOutput,"{\"type\":\"reply\",\"from\":%d,\"id\":%lu,\"end\":true,\"reply\":\"",to,id);

      setOutputHandler(&bitlashBuffer);

      ret = (int)doCommand(command);

      strcpy(bitlashOutput+strlen(bitlashOutput), "\"}\n");

      setOutputHandler(&bitlashFilter);

      leadSignal(bitlashOutput);

      free(bitlashOutput);

      return;
    }

    // we can only send one command at a time
    if(leadCommandTo)
    {
      // TODO we could stop reading the HQ socket in this mode and then never get a busy?
      return leadCommandError(to,id,"busy");
    }

    // send over mesh to recipient and cache id for any replies
    leadAnswerID = id;
    leadCommandTo = to;
    leadCommandChunks = (char*)malloc(strlen(command)+1);
    strcpy(leadCommandChunks,command);
    leadCommandChunksAt = 0;
    leadCommandRetries = 0;
    leadCommandChunk();
  }
}

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
    if(strlen(leadCommandChunks+leadCommandChunksAt) > 100)
      {
        leadCommandChunksAt += 100;
        leadCommandChunk();
        return; // don't free yet
      }
  } else {
    leadCommandRetries++;
    if(leadCommandRetries > 3)
    {
      Serial.print("error: ");
      Serial.println(req->status, HEX);
      leadCommandError(leadCommandTo, leadAnswerID, "no response");
    }else{
      Serial.println("RETRY");
      NWK_DataReq(req);
      return; // don't free yet
    }
  }
  leadCommandTo = 0;
  free(leadCommandChunks);
}

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk()
{
  int len = strlen(leadCommandChunks+leadCommandChunksAt);
  if(len > 100) len = 100;
  else len++; // null terminator at end
  leadCommandReq.dstAddr = leadCommandTo;
  leadCommandReq.dstEndpoint = 2;
  leadCommandReq.srcEndpoint = 3;
  leadCommandReq.options = NWK_OPT_ENABLE_SECURITY;
  leadCommandReq.data = (uint8_t*)(leadCommandChunks+leadCommandChunksAt);
  leadCommandReq.size = len;
  leadCommandReq.confirm = leadCommandChunkConfirm;
  NWK_DataReq(&leadCommandReq);
  //RgbLed.blinkCyan(200);
  Serial.print(leadCommandTo, DEC);
  Serial.print(" len ");
  Serial.print(len, DEC);
  Serial.println("->chunk");
}

// wrapper to send a chunk of JSON to the HQ
void leadSignal(char *json)
{
  if(!wifi.client.connected())
  {
    Serial.println("HQ offline, can't signal");
    Serial.println(json);
    return;
  }
  Serial.println("HQ signalling");
  Serial.println(json);
  wifi.client.write(json);
  wifi.client.flush();
}

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind) {
  bool end = false;
  int at;
  char sig[256];
  //RgbLed.blinkGreen(200);

  if(ind->options&NWK_IND_OPT_MULTICAST)
  {
    Serial.println("MULTICAST on wrong endpoint");
    return true;
  }

  Serial.println("Received answer");
  if(ind->data[ind->size-1] == 0)
  {
    end = true;
    ind->size--;
  }
  sprintf(sig,"{\"type\":\"reply\",\"id\":%d,\"from\":%d,\"data\":\"", leadAnswerID, ind->srcAddr);
  at = strlen(sig);
  memcpy(sig+at, ind->data, ind->size);
  sprintf(sig+at+ind->size, "\",\"end\":%s}\n",end?"true":"false");
  leadSignal(sig);

  return true;
}

// simple wrapper for the incoming channel announcements up to HQ
void leadAnnouncementSend(int chan, int from, char *message)
{
  char sig[256];
  sprintf(sig,"{\"type\":\"channel\",\"id\":%d,\"from\":%d,\"data\":\"%s\"}\n", chan, from, message);
  leadSignal(sig);
}

// mesh callback whenever another scout announces something on a channel
static bool leadAnnouncements(NWK_DataInd_t *ind) {
  RgbLed.blinkBlue(200);
  // be safe
  if(!ind->options&NWK_IND_OPT_MULTICAST) return true;

  Serial.print("MULTICAST");
  leadAnnouncementSend(ind->dstAddr, ind->srcAddr, (char*)ind->data);
  return true;
}

/////////////////////
// field scout stuff

// necessities to track state whenever we have to chunk up large bitlash answers to a command
int fieldAnswerTo = 0;
char *fieldAnswerChunks;
int fieldAnswerChunksAt;
int fieldAnswerRetries;
NWK_DataReq_t fieldAnswerReq;
void fieldAnswerChunk(void);

// chunk packet confirmation callback by mesh
static void fieldAnswerChunkConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
    if(strlen(fieldAnswerChunks+fieldAnswerChunksAt) > 100)
      {
        fieldAnswerChunksAt += 100;
        fieldAnswerChunk();
        return; // don't free yet
      }
  } else {
    fieldAnswerRetries++;
    if(fieldAnswerRetries > 3)
    {
      Serial.print("error: ");
      Serial.println(req->status, HEX);
    }else{
      Serial.println("RETRY");
      NWK_DataReq(req);
      return; // don't free yet
    }
  }
  fieldAnswerTo = 0;
  free(fieldAnswerChunks);
}

// send the first/next chunk of the answer back
static void fieldAnswerChunk()
{
  int len = strlen(fieldAnswerChunks+fieldAnswerChunksAt);
  if(len > 100) len = 100;
  else len++; // null terminator at end
  fieldAnswerReq.dstAddr = fieldAnswerTo;
  fieldAnswerReq.dstEndpoint = 3;
  fieldAnswerReq.srcEndpoint = 2;
  fieldAnswerReq.options = NWK_OPT_ENABLE_SECURITY;
  fieldAnswerReq.data = (uint8_t*)(fieldAnswerChunks+fieldAnswerChunksAt);
  fieldAnswerReq.size = len;
  fieldAnswerReq.confirm = fieldAnswerChunkConfirm;
  NWK_DataReq(&fieldAnswerReq);
  //RgbLed.blinkCyan(200);
  Serial.print(fieldAnswerTo, DEC);
  Serial.print(" len ");
  Serial.print(len, DEC);
  Serial.println("->chunk");
}

// necessities to buffer up large incoming commands
char *fieldCommand = NULL;
int fieldCommandLen = 0;

// mesh callback for handling incoming commands
static bool fieldCommands(NWK_DataInd_t *ind) {
  int total, ret;
  RgbLed.blinkGreen(200);

  Serial.print("Received command");
  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);
  Serial.print("  ");
  Serial.print("rssi: ");
  Serial.print(ind->rssi, DEC);
  Serial.println();

  if(fieldAnswerTo)
  {
    Serial.println("can't receive command while sending answer");
    return false;
  }

  // commands may be larger than one packet, copy and buffer up
  total = fieldCommandLen + ind->size;
  fieldCommand = (char*)realloc(fieldCommand, total);
  if(!fieldCommand) return false; // TODO we need to restart or something, no memory
  memcpy(fieldCommand+fieldCommandLen,ind->data,ind->size);
  fieldCommandLen = total;
  // when null terminated, do the message
  if(fieldCommand[fieldCommandLen-1] != 0){
    Serial.println("waiting for more");
    return true;
  }

  // run the command and chunk back the results
  setOutputHandler(&bitlashBuffer);
  bitlashOutput = (char*)malloc(1);
  bitlashOutput[0] = 0;
  Serial.print("running command ");
  Serial.println(fieldCommand);
  ret = (int)doCommand(fieldCommand);
  Serial.print("got result ");
  Serial.println(ret);
  setOutputHandler(&bitlashFilter);
  fieldCommandLen = 0;

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
  fieldAnswerChunks = bitlashOutput;
  fieldAnswerChunksAt = 0;
  fieldAnswerRetries = 0;
  fieldAnswerChunk();

  return true;
}

// can only send one at a time, locks up!
bool announcing = false;
static void fieldAnnounceConfirm(NWK_DataReq_t *req) {
  announcing = false;
}

// check a line of bitlash output for any announcements and send them
void fieldAnnounce(char *line)
{
  char *message;
  int chan, len;

  // any lines looking like "CHAN:4 message" will send "message" to channel 4
  if(strncmp("CH4N:",line,5) != 0) return;
  message = strchr(line,' ');
  if(!message) return;
  *message = 0;
  message++;
  chan = atoi(line+5);
  if(!chan || announcing) return;

  len = strlen(message);
  Serial.print("announcing to ");
  Serial.print(chan, DEC);
  Serial.print(" ");
  Serial.println(message);

  // when lead scout, shortcut
  if (isLeadScout) {
    return leadAnnouncementSend(chan, whoami, message);
  }

  Scout.meshJoinGroup(chan); // must be joined to send
  fieldAnnounceReq.dstAddr = chan;
  fieldAnnounceReq.dstEndpoint = 4;
  fieldAnnounceReq.srcEndpoint = whoami;
  fieldAnnounceReq.options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  fieldAnnounceReq.data = (uint8_t*)message;
  fieldAnnounceReq.size = len+1;
  fieldAnnounceReq.confirm = fieldAnnounceConfirm;
  announcing = true;
  NWK_DataReq(&fieldAnnounceReq);
  //RgbLed.blinkGreen(200);
}


/****************************\
 *      EVENT HANDLERS      *
\****************************/
void digitalPinEventHandler(uint8_t pin, uint8_t value) {
  uint32_t time = millis();

  if (findscript("event.digital")) {
    String callback = "event.digital(" + String(pin) + "," + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }

  if (Scout.eventVerboseOutput) {
    Serial.print("Digital pin event handler took ");
    Serial.print(millis() - time);
    Serial.println("ms");
    Serial.println();
  }
}

void analogPinEventHandler(uint8_t pin, uint16_t value) {
  if (findscript("event.analog")) {
    String callback = "event.analog(" + String(pin) + "," + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

void batteryPercentageEventHandler(uint8_t value) {
  if (findscript("event.percent")) {
    String callback = "event.percent(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

void batteryVoltageEventHandler(uint8_t value) {
  if (findscript("event.voltage")) {
    String callback = "event.voltage(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

void batteryChargingEventHandler(uint8_t value) {
  if (findscript("event.charging")) {
    String callback = "event.charging(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

void batteryAlarmTriggeredEventHandler(uint8_t value) {
  if (findscript("event.batteryalarm")) {
    String callback = "event.batteryalarm(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

void temperatureEventHandler(uint8_t value) {
  if (findscript("event.temperature")) {
    String callback = "event.temperature(" + String(value) + ")";
    char buf[24];
    callback.toCharArray(buf, callback.length()+1);
    doCommand(buf);
  }
}

// watches bitlash output for channel announcements
void bitlashFilter(byte b) {
  static char buf[101];
  static int offset = 0;

  Serial.write(b); // cc to serial
  if(b == '\r') return; // skip CR

  // newline or max len announces and resets
  if(b == '\n' || offset == 100)
  {
    buf[offset] = 0;
    fieldAnnounce(buf);
    offset=0;
    return;
  }
  if (b == '"')
  {
    buf[offset++] = '\\';
    b = '"';
  }
  buf[offset] = b;
  offset++;
}

// buffers bitlash output from a command
void bitlashBuffer(byte b) {
  int len;

  Serial.write(b); // cc to serial
  if(b == '\r') return; // skip CR

  len = strlen(bitlashOutput);
  bitlashOutput = (char*)realloc(bitlashOutput, len+3); // up to 2 bytes w/ escaping, and the null term

  // escape newlines, quotes, and slashes
  if(b == '\n')
  {
    bitlashOutput[len++] = '\\';
    b = 'n';
  }
  if(b == '"')
  {
    bitlashOutput[len++] = '\\';
    b = '"';
  }
  if(b == '\\')
  {
    bitlashOutput[len++] = '\\';
    b = '\\';
  }
  bitlashOutput[len] = b;
  bitlashOutput[len+1] = 0;
}