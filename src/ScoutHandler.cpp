#include <Arduino.h>
#include <ScoutHandler.h>
#include <Shell.h>
#include <Scout.h>
#include "backpack-bus/PBBP.h"
#include "backpacks/wifi/WiFiBackpack.h"
extern "C" {
#include <js0n.h>
#include <j0g.h>
#include "key/key.h"
#include "lwm/sys/sysTimer.h"
}

#define container_of(ptr, type, member) ({ \
                const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                (type *)( (char *)__mptr - offsetof(type,member) );})

static bool hqVerboseOutput;

static char *fieldCommand = NULL;
static int fieldCommandLen = 0;
static int fieldAnswerTo = 0;
static char *fieldAnswerChunks;
static int fieldAnswerChunksAt;
static int fieldAnswerRetries;
static NWK_DataReq_t fieldAnswerReq;

// mesh callback for handling incoming commands
static bool fieldCommands(NWK_DataInd_t *ind);

// chunk packet confirmation callback by mesh
static void fieldAnswerChunkConfirm(NWK_DataReq_t *req);

// send the first/next chunk of the answer back and confirm
static void fieldAnswerChunk();

// mesh callback whenever another scout announces something on a channel
static bool fieldAnnouncements(NWK_DataInd_t *ind);

// simple wrapper for the incoming channel announcements up to HQ
static void leadAnnouncementSend(uint16_t chan, uint16_t from, char *message);

// necessities for tracking state when chunking up a large command into mesh requests
static int leadCommandTo = 0;
static char *leadCommandChunks;
static int leadCommandChunksAt;
static int leadCommandRetries;
static NWK_DataReq_t leadCommandReq;
static void leadCommandChunk(void);
static int leadAnswerID = 0;

static bool leadAnswers(NWK_DataInd_t *ind);
static void leadSignal(char *json);
static void leadIncoming(char *packet, unsigned short *index);

// this is called on the main loop to try to (re)connect to the HQ
static void leadHQHandle(void);

// process a packet from HQ
static void leadIncoming(char *packet, unsigned short *index);

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req);

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk();

// wrapper to send a chunk of JSON to the HQ
static void leadSignal(char *json);

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind);


PinoccioScoutHandler::PinoccioScoutHandler() { }

PinoccioScoutHandler::~PinoccioScoutHandler() { }

void PinoccioScoutHandler::setup() {
  if (Scout.isLeadScout()) {
    Scout.wifi.setup();
    Scout.wifi.autoConnectHq();

    Scout.meshListen(3, leadAnswers);
    Scout.meshJoinGroup(0xBEEF); // our internal reporting channel
    Scout.meshJoinGroup(0); // reports to HQ
  } else {
    Scout.meshListen(2, fieldCommands);
  }

  // join a set of groups to listen for announcements
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

void PinoccioScoutHandler::setVerbose(bool flag) {
  hqVerboseOutput = flag;
}

static bool fieldCommands(NWK_DataInd_t *ind) {
  int total, ret;
//  RgbLed.blinkGreen(200);

  if (hqVerboseOutput) {
    sp(F("Received command"));
    sp(F("lqi: "));
    sp(ind->lqi);
    sp(F("  "));
    sp(F("rssi: "));
    speol(ind->rssi);
  }

  if (fieldAnswerTo) {
    if (hqVerboseOutput) {
      speol(F("can't receive command while sending answer"));
    }
    return false;
  }

  // commands may be larger than one packet, copy and buffer up
  total = fieldCommandLen + ind->size;

  fieldCommand = (char*)realloc(fieldCommand, total);
  if (!fieldCommand) {
    return false; // TODO we need to restart or something, no memory
  }

  memcpy(fieldCommand + fieldCommandLen, ind->data, ind->size);
  fieldCommandLen = total;

  // when null terminated, do the message
  if (fieldCommand[fieldCommandLen-1] != 0) {
    if (hqVerboseOutput) {
      speol(F("waiting for more"));
    }
    return true;
  }

  // run the command and chunk back the results
  if (!prepareBitlashBuffer()) return false;
  setOutputHandler(&bitlashBuffer);

  if (hqVerboseOutput) {
    sp(F("running command "));
    speol(fieldCommand);
  }

  ret = (int)doCommand(fieldCommand);
  if (hqVerboseOutput) {
    sp(F("got result "));
    speol(ret);
  }

  setOutputHandler(&bitlashFilter);
  fieldCommandLen = 0;

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
  // "Steal" the bitlashOutput buffer and set it to NULL to prevent
  // double free
  fieldAnswerChunks = Shell.bitlashOutput;
  Shell.bitlashOutput = NULL;
  fieldAnswerChunksAt = 0;
  fieldAnswerRetries = 0;
  fieldAnswerChunk();

  return true;
}

static void fieldAnswerChunkConfirm(NWK_DataReq_t *req) {
  if (hqVerboseOutput) {
    sp(F("  Message confirmation - "));
  }
  if (req->status == NWK_SUCCESS_STATUS) {
    if (hqVerboseOutput) {
      speol(F("success"));
    }
    if (strlen(fieldAnswerChunks + fieldAnswerChunksAt) > 100) {
      fieldAnswerChunksAt += 100;
      fieldAnswerChunk();
      return; // don't free yet
    }
  } else {
    fieldAnswerRetries++;
    if (fieldAnswerRetries > 3) {
      if (hqVerboseOutput) {
        sp(F("error: "));
        speol(req->status);
      }
    } else {
      if (hqVerboseOutput) {
        speol(F("RETRY"));
      }
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

  if (hqVerboseOutput) {
    sp(fieldAnswerTo);
    sp(F(" len "));
    sp(len);
    speol(F("->chunk"));
  }
}

static void announceConfirm(NWK_DataReq_t *req) {
  if (req->status != NWK_SUCCESS_STATUS && hqVerboseOutput) {
    sp(F("Mesh announce failed: "));
    speol(req->status);
  }
  free(req->data);
  free(req);
}

void PinoccioScoutHandler::announce(uint16_t group, char *message) {
  if (hqVerboseOutput) {
    sp(F("announcing to "));
    sp(group);
    sp(F(" "));
    speol(message);
  }

  // when lead scout, shortcut
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(group, Scout.getAddress(), message);
    // Don't broadcast HQ commands over the network if we are a lead
    // scout
    if (group == 0xBEEF)
      return;
  }

  char *data = strdup(message);
  if (!data) {
    return;
  }

  struct NWK_DataReq_t *r = (struct NWK_DataReq_t*)malloc(sizeof(struct NWK_DataReq_t));
  if (!r) {
    free(data);
    return;
  }

  Scout.meshJoinGroup(group); // must be joined to send
  memset(r, 0, sizeof(struct NWK_DataReq_t));
  r->dstAddr = group;
  r->dstEndpoint = 4;
  r->srcEndpoint = Scout.getAddress();
  r->options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  r->data = (uint8_t*)data;
  r->size = strlen(data)+1;
  r->confirm = announceConfirm;
  NWK_DataReq(r);
}

static bool fieldAnnouncements(NWK_DataInd_t *ind) {
  char callback[32], *data = (char*)ind->data;
  // be safe
  if (!ind->options & NWK_IND_OPT_MULTICAST) {
    return true;
  }

  if (hqVerboseOutput) {
    sp(F("multicast in "));
    sp(ind->dstAddr);
    speol();
  }
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(ind->dstAddr, ind->srcAddr, data);
  }
  if (ind->dstAddr == 0xBEEF || strlen(data) <3 || data[0] != '[') {
    return false;
  }

  int keys[10];
  keyLoad((char*)ind->data, keys, millis());

  // run the Bitlash callback function, if defined
  snprintf(callback, sizeof(callback), "event.group%d", ind->dstAddr);
  if (findscript(callback)) {
    char buf[128];
    snprintf(buf, sizeof(buf), "event.group%d(%d", ind->dstAddr, ind->srcAddr);
    for (int i=2; i<=keys[0]; i++) {
      snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ",%d", keys[i]);
    }
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ")");
    doCommand(buf);
  }

  return true;
}

// just store one converted report at a time
char reportJson[256];
char *report2json(char *in) {
  char *keys, *vals, report[100];
  unsigned short ir[16], ik[32], iv[32], i;

  // copy cuz we edit it
  memcpy(report, in, 100);

  // parse this and humanize
  js0n((unsigned char*)report, strlen(report), ir, 16);
  if (!*ir) {
    return NULL;
  }

  snprintf(reportJson, sizeof(reportJson), "{\"type\":\"%s\"", keyGet(atoi(j0g_safe(0, report, ir))));
  keys = report + ir[2];
  js0n((unsigned char*)keys, ir[3], ik, 32);
  if (!*ik) {
    return NULL;
  }
  vals = report+ir[4];

  js0n((unsigned char*)vals, ir[5], iv, 32);
  if (!*iv) {
    return NULL;
  }

  for (i=0; ik[i]; i+=2) {
    snprintf(reportJson + strlen(reportJson), sizeof(reportJson) - strlen(reportJson), ",\"%s\":", keyGet(atoi(j0g_safe(i, keys, ik))));
    if (vals[iv[i]-1] == '"') {
      iv[i]--;
      iv[i+1]+=2;
    }
    *(vals+iv[i]+iv[i+1]) = 0;
    snprintf(reportJson + strlen(reportJson), sizeof(reportJson) - strlen(reportJson), "%s", vals + iv[i]);
  }

  snprintf(reportJson + strlen(reportJson), sizeof(reportJson) - strlen(reportJson), "}");
  return reportJson;
}


static void leadAnnouncementSend(uint16_t group, uint16_t from, char *message) {
  char *report;
  // reports are expected to be json objects
  if (group == 0xBEEF) {
    const char *json = report2json(message);
    size_t len = strlen(json) + 128;
    report = (char*)malloc(len);
    if (!report) return;
    snprintf(report, len, "{\"type\":\"report\",\"from\":%d,\"report\":%s}\n", from, json);
  } else if (group == 0) {
    size_t len = strlen(message) + 128;
    report = (char*)malloc(len);
    if (!report) return;
    snprintf(report, len, "{\"type\":\"announce\",\"from\":%d,\"announce\":%s}\n", from, message);
  } else {
    return;
  }
  leadSignal(report);
  free(report);
}

// [3,[0,1,2],[v,v,v]]
char *PinoccioScoutHandler::report(char *report) {
  Scout.handler.announce(0xBEEF, report);
  return report2json(report);
}

////////////////////
// lead scout stuff

void leadHQConnect() {
  char auth[256], token[33];

  if (Scout.wifi.client.connected()) {
    Pinoccio.getHQToken(token);
    token[32] = 0;
    snprintf(auth, sizeof(auth), "{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    leadSignal(auth);
  } else {
    if (hqVerboseOutput) {
      speol(F("server unvailable"));
    }
  }
}

// this is called on the main loop to try to (re)connect to the HQ
void leadHQHandle(void) {
  static char *buffer = NULL;
  uint8_t block[128];
  char *nl;
  int rsize, len;
  unsigned short index[32]; // <10 keypairs in the incoming json

  // only continue if new data to read
  if (!Scout.wifi.client.available()) {
    return;
  }

  // check to initialize our read buffer
  if (!buffer) {
    buffer = (char*)malloc(1);
    if (!buffer) return;
    *buffer = 0;
  }

  // get all waiting data and look for packets
  while (rsize = Scout.wifi.client.read(block, sizeof(block))) {
    len = strlen(buffer);

    // process chunk of incoming data
    buffer = (char*)realloc(buffer, len + rsize + 1);
    if (!buffer) {
      return; // TODO, realloc error, need to restart?
    }
    memcpy(buffer + len, block, rsize);
    buffer[len + rsize] = 0; // null terminate

    // look for a packet
    if (hqVerboseOutput) {
      sp(F("looking for packet in: "));
      speol(buffer);
    }
    nl = strchr(buffer, '\n');
    if (!nl) {
      continue;
    }

    // null terminate just the packet and process it
    *nl = 0;
    j0g(buffer, index, 32);
    if (*index) {
      leadIncoming(buffer, index);
    } else {
      if (hqVerboseOutput) {
        speol(F("JSON parse failed"));
      }
    }

    // advance buffer and resize, minimum is just the buffer end null
    nl++;
    len = strlen(nl);
    memmove(buffer, nl, len+1);
    buffer = (char*)realloc(buffer, len+1); // shrink
  }
}

// when we can't process a command for some internal reason
void leadCommandError(int from, int id, const char *reason) {
  size_t len = strlen(reason) + 128;
  char *err = (char*)malloc(len);
  if (!err) return;
  snprintf(err, len, "{\"type\":\"reply\",\"from\":%d,\"id\":%d,\"err\":true,\"reply\":\"%s\"}\n", from, id, reason);
  leadSignal(err);
  free(err);
}

// process a packet from HQ
void leadIncoming(char *packet, unsigned short *index) {
  char *type, *command;
  uint16_t to;
  unsigned long id;

  type = j0g_str("type", packet, index);
  if (hqVerboseOutput) {
    speol(type);
  }

  if (strcmp(type, "online") == 0) {
    Shell.allReportHQ();
  }

  if (strcmp(type, "command") == 0) {
    to = atoi(j0g_str("to", packet, index));
    id = strtoul(j0g_str("id", packet, index), NULL, 10);
    command = j0g_str("command", packet, index);
    if (strlen(j0g_str("to", packet, index)) == 0 || !id || !command) {
      if (hqVerboseOutput) {
        speol(F("invalid command, requires to, id, command"));
        sp(F("to: "));
        speol(to);
        sp(F("id: "));
        speol(id);
        sp(F("command: "));
        speol(command);
      }
      return;
    }

    // handle internal ones first
    if (to == Scout.getAddress()) {
      if (!prepareBitlashBuffer()) return;
      setOutputHandler(&bitlashBuffer);
      doCommand(command);
      setOutputHandler(&bitlashFilter);

      size_t len = strlen(Shell.bitlashOutput) + 255;
      char *report = (char*)malloc(len);
      if (!report) return;
      snprintf(report, len, "{\"type\":\"reply\",\"from\":%d,\"id\":%lu,\"end\":true,\"reply\":\"%s\"}\n", to, id, Shell.bitlashOutput);
      leadSignal(report);
      free(report);

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
    leadCommandChunks = strdup(command);
    leadCommandChunksAt = 0;
    leadCommandRetries = 0;
    leadCommandChunk();
  }
}

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req) {
  if (hqVerboseOutput) {
    sp(F("  Message confirmation - "));
  }
  if (req->status == NWK_SUCCESS_STATUS) {
    if (hqVerboseOutput) {
      speol(F("success"));
    }
    if (strlen(leadCommandChunks+leadCommandChunksAt) > 100) {
      leadCommandChunksAt += 100;
      leadCommandChunk();
      return; // don't free yet
    }
  } else {
    leadCommandRetries++;
    if (leadCommandRetries > 3) {
      if (hqVerboseOutput) {
        sp(F("error: "));
        speol(req->status);
      }
      leadCommandError(leadCommandTo, leadAnswerID, "no response");
    } else {
      if (hqVerboseOutput) {
        speol(F("RETRY"));
      }
      NWK_DataReq(req);
      return; // don't free yet
    }
  }
  leadCommandTo = 0;
  free(leadCommandChunks);
}

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk() {
  int len = strlen(leadCommandChunks + leadCommandChunksAt);
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

  if (hqVerboseOutput) {
    sp(leadCommandTo);
    sp(F(" len "));
    sp(len);
    speol(F("->chunk"));
  }
}

// wrapper to send a chunk of JSON to the HQ
void leadSignal(char *json) {
  if (!Scout.wifi.client.connected()) {
    if (hqVerboseOutput) {
      speol(F("HQ offline, can't signal"));
      speol(json);
    }
    return;
  }
  if (hqVerboseOutput) {
    speol(F("HQ signalling"));
    speol(json);
  }

  Scout.wifi.client.write(json);
  Scout.wifi.client.flush();
}

// called whenever another scout sends an answer back to us
bool leadAnswers(NWK_DataInd_t *ind) {
  bool end = false;
  int at;
  char sig[256];

  if (ind->options & NWK_IND_OPT_MULTICAST) {
    if (hqVerboseOutput) {
      speol(F("MULTICAST on wrong endpoint"));
    }
    return true;
  }

  if (hqVerboseOutput) {
    speol(F("Received answer"));
  }
  if (ind->data[ind->size-1] == 0) {
    end = true;
    ind->size--;
  }
  snprintf(sig, sizeof(sig),"{\"type\":\"reply\",\"id\":%d,\"from\":%d,\"reply\":\"", leadAnswerID, ind->srcAddr);
  at = strlen(sig);
  memcpy(sig+at, ind->data, ind->size);
  snprintf(sig+at+ind->size, sizeof(sig) - at - ind->size, "\",\"end\":%s}\n",end ? "true" : "false");
  leadSignal(sig);

  return true;
}
