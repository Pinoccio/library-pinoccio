#include <Arduino.h>
#include <ScoutHandler.h>
#include <Shell.h>
#include <Scout.h>
#include "backpack-bus/PBBP.h"
#include "backpacks/wifi/WiFiBackpack.h"
#include "util/StringBuffer.h"
#include "util/PrintToString.h"
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

static StringBuffer fieldCommand(0, 16);
static int fieldAnswerTo = 0;
static char *fieldAnswerChunks;
static int fieldAnswerChunksAt;
static int fieldAnswerRetries;
static NWK_DataReq_t fieldAnswerReq;

StringBuffer fieldCommandOutput;

// mesh callback for handling incoming commands
static bool fieldCommands(NWK_DataInd_t *ind);

// chunk packet confirmation callback by mesh
static void fieldAnswerChunkConfirm(NWK_DataReq_t *req);

// send the first/next chunk of the answer back and confirm
static void fieldAnswerChunk();

// mesh callback whenever another scout announces something on a channel
static bool fieldAnnouncements(NWK_DataInd_t *ind);

// simple wrapper for the incoming channel announcements up to HQ
static void leadAnnouncementSend(uint16_t chan, uint16_t from, const char *message);

// necessities for tracking state when chunking up a large command into mesh requests
static int leadCommandTo = 0;
StringBuffer leadCommandChunks;
static int leadCommandChunksAt;
static int leadCommandRetries;
static NWK_DataReq_t leadCommandReq;
static void leadCommandChunk(void);
static int leadAnswerID = 0;

// this is called on the main loop to try to (re)connect to the HQ
static void leadHQHandle(void);

// process a packet from HQ
static void leadIncoming(const char *packet, size_t len, unsigned short *index);

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req);

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk();

// wrapper to send a chunk of JSON to the HQ
static void leadSignal(const char *json);

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
  int ret;
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

  if (!fieldCommand.concat((const char*)ind->data, ind->size)) {
    return false; // TODO we need to restart or something, no memory
  }

  // when null terminated, do the message
  if (fieldCommand[fieldCommand.length() - 1] != '\0') {
    if (hqVerboseOutput) {
      speol(F("waiting for more"));
    }
    return true;
  }

  // run the command and chunk back the results
  setOutputHandler(&printToString<&fieldCommandOutput>);

  if (hqVerboseOutput) {
    sp(F("running command "));
    speol(fieldCommand);
  }

  // TODO: Once bitlash fixes const-correctness, this and similar casts
  // should be removed.
  ret = (int)doCommand(const_cast<char *>(fieldCommand.c_str()));

  if (hqVerboseOutput) {
    sp(F("got result "));
    speol(ret);
  }

  setOutputHandler(&bitlashFilter);

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
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
    if (fieldCommandOutput.length() - fieldAnswerChunksAt > 100) {
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
  // Free memory used by Stringbuffer
  fieldCommandOutput = (char*)NULL;
}

static void fieldAnswerChunk() {
  int len = fieldCommandOutput.length() - fieldAnswerChunksAt;
  if (len > 100) {
    len = 100;
  } else {
    len++; // null terminator at end
  }

  fieldAnswerReq.dstAddr = fieldAnswerTo;
  fieldAnswerReq.dstEndpoint = 3;
  fieldAnswerReq.srcEndpoint = 2;
  fieldAnswerReq.options = NWK_OPT_ENABLE_SECURITY;
  fieldAnswerReq.data = (uint8_t*)fieldCommandOutput.c_str() + fieldAnswerChunksAt;
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

void PinoccioScoutHandler::announce(uint16_t group, const String& message) {
  if (hqVerboseOutput) {
    sp(F("announcing to "));
    sp(group);
    sp(F(" "));
    speol(message);
  }

  // when lead scout, shortcut
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(group, Scout.getAddress(), message.c_str());
    // Don't broadcast HQ commands over the network if we are a lead
    // scout
    if (group == 0xBEEF)
      return;
  }

  // TODO: Allocate the data and request pointers in a single malloc?
  char *data = strdup(message.c_str());
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
  r->size = message.length() + 1; // include NUL byte
  r->confirm = announceConfirm;
  NWK_DataReq(r);
}

static bool fieldAnnouncements(NWK_DataInd_t *ind) {
  char *data = (char*)ind->data;
  // be safe
  if (!ind->options & NWK_IND_OPT_MULTICAST) {
    return true;
  }

  if (hqVerboseOutput) {
    sp(F("multicast in "));
    speol(ind->dstAddr);
  }
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(ind->dstAddr, ind->srcAddr, data);
  }
  if (ind->dstAddr == 0xBEEF || strlen(data) < 3 || data[0] != '[') {
    return false;
  }

  int keys[10];
  keyLoad((char*)ind->data, keys, millis());

  // run the Bitlash callback function, if defined
  StringBuffer callback(20);
  callback.appendSprintf("event.group%d", ind->dstAddr);
  if (findscript(const_cast<char*>(callback.c_str()))) {
    StringBuffer buf(64, 16);
    buf.appendSprintf("event.group%d(%d", ind->dstAddr, ind->srcAddr);
    for (int i=2; i<=keys[0]; i++) {
      buf.appendSprintf(",%d", keys[i]);
    }
    buf += "}";
    doCommand(const_cast<char*>(buf.c_str()));
  }

  return true;
}

// just store one converted report at a time
StringBuffer report2json(const char *in) {
  char *keys, *vals;
  unsigned short ir[16], ik[32], iv[32], i;
  StringBuffer reportJson(100, 8);

  // copy cuz we edit it
  char *report = strdup(in);

  // parse this and humanize
  js0n((unsigned char*)report, strlen(report), ir, 16);
  if (!*ir) {
    free(report);
  }

  // TODO: Proper JSON escaping in this function
  reportJson.appendSprintf("{\"type\":\"%s\"", keyGet(atoi(j0g_safe(0, report, ir))));

  keys = report + ir[2];
  js0n((unsigned char*)keys, ir[3], ik, 32);
  if (!*ik) {
    free(report);
    reportJson = (char*)NULL;
    return reportJson;
  }
  vals = report+ir[4];

  js0n((unsigned char*)vals, ir[5], iv, 32);
  if (!*iv) {
    free(report);
    reportJson = (char*)NULL;
    return reportJson;
  }

  for (i=0; ik[i]; i+=2) {
    reportJson.appendSprintf(",\"%s\":", keyGet(atoi(j0g_safe(i, keys, ik))));

    if (vals[iv[i]-1] == '"') {
      iv[i]--;
      iv[i+1]+=2;
    }
    *(vals+iv[i]+iv[i+1]) = 0;
    reportJson.appendSprintf("%s", vals + iv[i]);
  }

  reportJson += "}";
  free(report);
  return reportJson;
}


static void leadAnnouncementSend(uint16_t group, uint16_t from, const char *message) {
  StringBuffer report(100, 8);

  // reports are expected to be json objects
  if (group == 0xBEEF) {
    report.appendSprintf("{\"type\":\"report\",\"from\":%d,\"report\":%s}\n", from, (report2json(message)).c_str());
  } else if (group == 0) {
    report.appendSprintf("{\"type\":\"announce\",\"from\":%d,\"announce\":%s}\n", from, message);
  } else {
    return;
  }
  leadSignal(report.c_str());
}

// [3,[0,1,2],[v,v,v]]
StringBuffer PinoccioScoutHandler::report(const String &report) {
  Scout.handler.announce(0xBEEF, report);
  return report2json(report.c_str());
}

////////////////////
// lead scout stuff

void leadHQConnect() {

  if (Scout.wifi.client.connected()) {
    char token[33];
    StringBuffer auth(64);
    Pinoccio.getHQToken(token);
    token[32] = 0;
    auth.appendSprintf("{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    leadSignal(auth.c_str());
  } else {
    if (hqVerboseOutput) {
      speol(F("server unvailable"));
    }
  }
}

// this is called on the main loop to process incoming data from HQ
static StringBuffer hqIncoming;
void leadHQHandle(void) {
  int rsize;
  unsigned short index[32]; // <10 keypairs in the incoming json

  // only continue if new data to read
  if (!Scout.wifi.client.available()) {
    return;
  }

  // Read a block of data and look for packets
  while ((rsize = hqIncoming.readClient(Scout.wifi.client, 128))) {
    int nl;
    while((nl = hqIncoming.indexOf('\n')) >= 0) {
     // look for a packet
      if (hqVerboseOutput) {
        sp(F("looking for packet in: "));
        speol(hqIncoming);
      }

      // Parse JSON up to the first newline
      if (!js0n((const unsigned char*)hqIncoming.c_str(), nl, index, 32)) {
        leadIncoming(hqIncoming.c_str(), nl, index);
      } else {
        if (hqVerboseOutput) {
          speol(F("JSON parse failed"));
        }
      }

      // Remove up to and including the newline
      hqIncoming.remove(0, nl + 1);
    }
  }
}

// when we can't process a command for some internal reason
void leadCommandError(int from, int id, const char *reason) {
  StringBuffer err(128);
  err.appendSprintf("{\"type\":\"reply\",\"from\":%d,\"id\":%d,\"err\":true,\"reply\":\"%s\"}\n", from, id, reason);
  leadSignal(err.c_str());
}

StringBuffer leadCommandOutput;

// process a packet from HQ
void leadIncoming(const char *packet, size_t len, unsigned short *index) {
  char *type, *command, *buffer;

  buffer = (char*)malloc(len);
  memcpy(buffer, packet, len);

  uint16_t to;
  unsigned long id;

  type = j0g_str("type", buffer, index);
  if (hqVerboseOutput) {
    speol(type);
  }

  if (strcmp(type, "online") == 0) {
    Shell.allReportHQ();
  }

  if (strcmp(type, "command") == 0) {
    to = atoi(j0g_str("to", buffer, index));
    id = strtoul(j0g_str("id", buffer, index), NULL, 10);
    command = j0g_str("command", buffer, index);
    if (strlen(j0g_str("to", buffer, index)) == 0 || !id || !command) {
      if (hqVerboseOutput) {
        speol(F("invalid command, requires to, id, command"));
        sp(F("to: "));
        speol(to);
        sp(F("id: "));
        speol(id);
        sp(F("command: "));
        speol(command);
      }
      free(buffer);
      return;
    }

    free(buffer);

    // handle internal ones first
    if (to == Scout.getAddress()) {
      setOutputHandler(&printToString<&leadCommandOutput>);
      doCommand(command);
      setOutputHandler(&bitlashFilter);

      StringBuffer report;
      report.appendSprintf("{\"type\":\"reply\",\"from\":%d,\"id\":%lu,\"end\":true,\"reply\":\"%s\"}\n", to, id, leadCommandOutput.c_str());
      leadSignal(report.c_str());
      leadCommandOutput = (char*)NULL;

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
    leadCommandChunks = command;
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
    if (leadCommandChunks.length() - leadCommandChunksAt > 100) {
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
  leadCommandChunks = (char*)NULL;
}

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk() {
  int len = leadCommandChunks.length() - leadCommandChunksAt;
  if (len > 100) {
    len = 100;
  } else {
    len++; // null terminator at end
  }

  leadCommandReq.dstAddr = leadCommandTo;
  leadCommandReq.dstEndpoint = 2;
  leadCommandReq.srcEndpoint = 3;
  leadCommandReq.options = NWK_OPT_ENABLE_SECURITY;
  leadCommandReq.data = (uint8_t*)leadCommandChunks.c_str() + leadCommandChunksAt;
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
void leadSignal(const char *json) {
  if (!Scout.wifi.client.connected()) {
    if (hqVerboseOutput) {
      speol(F("HQ offline, can't signal"));
      speol(json);
    }
    return;
  }
  if (hqVerboseOutput) {
    speol(F("Signalling HQ: "));
    speol(json);
  }

  Scout.wifi.client.write(json);
  Scout.wifi.client.flush();
}

// called whenever another scout sends an answer back to us
bool leadAnswers(NWK_DataInd_t *ind) {
  bool end = false;
  StringBuffer buf(256);

  if (ind->options & NWK_IND_OPT_MULTICAST) {
    if (hqVerboseOutput) {
      speol(F("MULTICAST on wrong endpoint"));
    }
    return true;
  }

  if (hqVerboseOutput) {
    sp(F("Received answer from Scout "));
    sp(ind->srcAddr);
    speol(F(":"));
  }
  if (ind->data[ind->size-1] == 0) {
    end = true;
    ind->size--;
  }
  buf.appendSprintf("{\"type\":\"reply\",\"id\":%d,\"from\":%d,\"reply\":", leadAnswerID, ind->srcAddr);
  buf.appendJsonString(ind->data, ind->size, true);
  buf.appendSprintf(",\"end\":%s}\n",end ? "true" : "false");
  leadSignal(buf.c_str());

  return true;
}
