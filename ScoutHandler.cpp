#include <Arduino.h>
#include <ScoutHandler.h>
#include <Shell.h>
#include <Scout.h>
#include <PBBP.h>
#include <utility/WiFiBackpack.h>
extern "C" {
#include <j0g.h>
#include "utility/sysTimer.h"
}

PinoccioScoutHandler::PinoccioScoutHandler() { }

PinoccioScoutHandler::~PinoccioScoutHandler() { }

void PinoccioScoutHandler::setup() {
  if (Scout.isLeadScout()) {
    Gainspan.connectEventHandler = hqConnectHandler;
    Gainspan.disconnectEventHandler = hqDisconnectHandler;

    Scout.enableBackpackVcc();
    Serial.print("Wi-Fi backpack connecting...");
    Scout.wifi.setup();
    Scout.wifi.init();
    Serial.println("Done");
    RgbLed.blinkGreen();

    Scout.meshListen(3, leadAnswers);
    Scout.meshJoinGroup(0xBEEF); // our internal reporting channel

    Serial.println("- Lead Scout ready -");
  } else {
    Scout.meshListen(2, fieldCommands);
    Serial.println("- Field Scout ready -");
  }

  // join all the "channels" to listen for announcements
  for (int i = 1; i < 10; i++) {
    Scout.meshJoinGroup(i);
  }

  Scout.meshListen(4, fieldAnnouncements);
  setOutputHandler(&bitlashFilter);
}

void PinoccioScoutHandler::loop() {
  if (Scout.isLeadScout()) {
    leadHQHandle();
  }
}

static void hqConnectHandler(uint8_t cid) {
  leadHQConnect(cid);
}

static void hqDisconnectHandler(uint8_t cid) {
  Serial.println("Disconnected from HQ");
}

static bool fieldCommands(NWK_DataInd_t *ind) {
  int total, ret;
  RgbLed.blinkGreen(200);

  sp("Received command");
  sp("lqi: ");
  sp(ind->lqi);
  sp("  ");
  sp("rssi: ");
  sp(ind->rssi);
  speol();

  if (fieldAnswerTo) {
    sp("can't receive command while sending answer");
    speol();
    return false;
  }

  // commands may be larger than one packet, copy and buffer up
  total = fieldCommandLen + ind->size;

  fieldCommand = (char*)realloc(fieldCommand, total);
  if (!fieldCommand) {
    return false; // TODO we need to restart or something, no memory
  }

  memcpy(fieldCommand+fieldCommandLen, ind->data, ind->size);
  fieldCommandLen = total;

  // when null terminated, do the message
  if (fieldCommand[fieldCommandLen-1] != 0) {
    Serial.println("waiting for more");
    return true;
  }

  // run the command and chunk back the results
  setOutputHandler(&bitlashBuffer);
  Shell.bitlashOutput = (char*)malloc(1);
  Shell.bitlashOutput[0] = 0;
  Serial.print("running command ");
  Serial.println(fieldCommand);

  ret = (int)doCommand(fieldCommand);
  Serial.print("got result ");
  Serial.println(ret);

  setOutputHandler(&bitlashFilter);
  fieldCommandLen = 0;

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
  fieldAnswerChunks = Shell.bitlashOutput;
  fieldAnswerChunksAt = 0;
  fieldAnswerRetries = 0;
  fieldAnswerChunk();

  return true;
}

static void fieldAnswerChunkConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
    if (strlen(fieldAnswerChunks + fieldAnswerChunksAt) > 100) {
      fieldAnswerChunksAt += 100;
      fieldAnswerChunk();
      return; // don't free yet
    }
  } else {
    fieldAnswerRetries++;
    if (fieldAnswerRetries > 3) {
      Serial.print("error: ");
      Serial.println(req->status, HEX);
    } else {
      Serial.println("RETRY");
      NWK_DataReq(req);
      return; // don't free yet
    }
  }
  fieldAnswerTo = 0;
  free(fieldAnswerChunks);
}

static void fieldAnswerChunk() {
  int len = strlen(fieldAnswerChunks+fieldAnswerChunksAt);
  if (len > 100) {
    len = 100;
  } else {
    len++; // null terminator at end
  }

  fieldAnswerReq.dstAddr = fieldAnswerTo;
  fieldAnswerReq.dstEndpoint = 3;
  fieldAnswerReq.srcEndpoint = 2;
  fieldAnswerReq.options = NWK_OPT_ENABLE_SECURITY;
  fieldAnswerReq.data = (uint8_t*)(fieldAnswerChunks + fieldAnswerChunksAt);
  fieldAnswerReq.size = len;
  fieldAnswerReq.confirm = fieldAnswerChunkConfirm;
  NWK_DataReq(&fieldAnswerReq);

  //RgbLed.blinkCyan(200);
  Serial.print(fieldAnswerTo, DEC);
  Serial.print(" len ");
  Serial.print(len, DEC);
  Serial.println("->chunk");
}

static void fieldAnnounceConfirm(NWK_DataReq_t *req) {
  isAnnouncing = false;
}

void PinoccioScoutHandler::fieldAnnounce(uint16_t chan, char *message) {
  int len = strlen(message);

  if (isAnnouncing) {
    return;
  }

  Serial.print("announcing to 0x");
  Serial.print(chan, HEX);
  Serial.print(" ");
  Serial.println(message);

  // when lead scout, shortcut
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(chan, Scout.getAddress(), message);
  }

  Scout.meshJoinGroup(chan); // must be joined to send
  fieldAnnounceReq.dstAddr = chan;
  fieldAnnounceReq.dstEndpoint = 4;
  fieldAnnounceReq.srcEndpoint = Scout.getAddress();
  fieldAnnounceReq.options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  fieldAnnounceReq.data = (uint8_t*)message;
  fieldAnnounceReq.size = len+1;
  fieldAnnounceReq.confirm = fieldAnnounceConfirm;
  isAnnouncing = true;
  NWK_DataReq(&fieldAnnounceReq);
}

static bool fieldAnnouncements(NWK_DataInd_t *ind) {
  char callback[32];
  RgbLed.blinkBlue(200);
  // be safe
  if (!ind->options & NWK_IND_OPT_MULTICAST) {
    return true;
  }

  Serial.print("MULTICAST");
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(ind->dstAddr, ind->srcAddr, (char*)ind->data);
  }

  // run the Bitlash callback function, if defined
  sprintf(callback,"event.channel%d",ind->dstAddr);
  if (findscript(callback)) {
    doCommand(callback);
  }

  return true;
}

static void leadAnnouncementSend(int chan, int from, char *message) {
  char sig[256];
  // reports are expected to be json objects
  if(chan == 0xBEEF) sprintf(sig,"{\"type\":\"report\",\"from\":%d,\"report\":%s}\n", from, message);
  else sprintf(sig,"{\"type\":\"channel\",\"id\":%d,\"from\":%d,\"data\":\"%s\"}\n", chan, from, message);
  leadSignal(sig);
}

////////////////////
// lead scout stuff

void leadHQConnect(uint8_t cid) {
  char auth[256], token[33];

  if (Scout.wifi.client.connected()) {
    Pinoccio.getHQToken(token);
    token[32] = 0;
    sprintf(auth,"{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    leadSignal(auth);
  } else {
    Serial.println("server unvailable");
  }
}

// this is called on the main loop to try to (re)connect to the HQ
void leadHQHandle(void) {
  static char *buffer = NULL;
  uint8_t block[128];
  char *nl;
  int rsize, len, i;
  unsigned short index[32]; // <10 keypairs in the incoming json

  // only continue if new data to read
  if (!Scout.wifi.client.available()) {
    return;
  }

  // check to initialize our read buffer
  if(!buffer) {
    buffer = (char*)malloc(1);
    *buffer = 0;
  }

  // get all waiting data and look for packets
  while(rsize = Scout.wifi.client.read(block, 256)){
    len = strlen(buffer);

    // process chunk of incoming data
    buffer = (char*)realloc(buffer, len+rsize+1);
    if (!buffer) {
      return; // TODO, realloc error, need to restart?
    }
    memcpy(buffer+len, block, rsize);
    buffer[len+rsize] = 0; // null terminate

    // look for a packet
    Serial.print("looking for packet in: ");
    Serial.println(buffer);
    nl = strchr(buffer, '\n');
    if (!nl) {
      continue;
    }

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
void leadCommandError(int from, int id, char *reason) {
  char *err;
  err = (char*)malloc(strlen(reason)+128);
  sprintf(err,"{\"type\":\"reply\",\"from\":%d,\"id\":%d,\"err\":true,\"reply\":\"%s\"}\n",from,id,reason);
  leadSignal(err);
  free(err);
}

// process a packet from HQ
void leadIncoming(char *packet, unsigned short *index) {
  char *type, *command;
  int to, ret, len;
  unsigned long id;

  type = j0g_str("type", packet, index);
  Serial.println(type);

  if (strcmp(type, "online") == 0) {
    // TODO anything hard-coded to do once confirmed online?
  }

  if (strcmp(type, "command") == 0) {
    to = atoi(j0g_str("to", packet, index));
    id = strtoul(j0g_str("id", packet, index), NULL, 10);
    command = j0g_str("command", packet, index);
    if (strlen(j0g_str("to", packet, index)) == 0 || !id || !command) {
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
    if (to == Scout.getAddress()) {
      Shell.bitlashOutput = (char*)malloc(255);

      sprintf(Shell.bitlashOutput,"{\"type\":\"reply\",\"from\":%d,\"id\":%lu,\"end\":true,\"reply\":\"",to,id);
      setOutputHandler(&bitlashBuffer);
      ret = (int)doCommand(command);
      strcpy(Shell.bitlashOutput + strlen(Shell.bitlashOutput), "\"}\n");
      setOutputHandler(&bitlashFilter);
      leadSignal(Shell.bitlashOutput);
      free(Shell.bitlashOutput);
      return;
    }

    // we can only send one command at a time
    if (leadCommandTo) {
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
    if (strlen(leadCommandChunks+leadCommandChunksAt) > 100) {
      leadCommandChunksAt += 100;
      leadCommandChunk();
      return; // don't free yet
    }
  } else {
    leadCommandRetries++;
    if (leadCommandRetries > 3) {
      Serial.print("error: ");
      Serial.println(req->status, HEX);
      leadCommandError(leadCommandTo, leadAnswerID, "no response");
    } else {
      Serial.println("RETRY");
      NWK_DataReq(req);
      return; // don't free yet
    }
  }
  leadCommandTo = 0;
  free(leadCommandChunks);
}

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk() {
  int len = strlen(leadCommandChunks+leadCommandChunksAt);
  if (len > 100) {
    len = 100;
  } else {
    len++; // null terminator at end
  }

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
void leadSignal(char *json) {
  if (!Scout.wifi.client.connected()) {
    Serial.println("HQ offline, can't signal");
    Serial.println(json);
    return;
  }
  Serial.println("HQ signalling");
  Serial.println(json);
  Scout.wifi.client.write(json);
  Scout.wifi.client.flush();
}

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind) {
  bool end = false;
  int at;
  char sig[256];
  //RgbLed.blinkGreen(200);

  if (ind->options&NWK_IND_OPT_MULTICAST) {
    Serial.println("MULTICAST on wrong endpoint");
    return true;
  }

  Serial.println("Received answer");
  if (ind->data[ind->size-1] == 0) {
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

