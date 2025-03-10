/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <Scout.h>
#include <Shell.h>
#include "SleepHandler.h"
#include "backpack-bus/PBBP.h"
#include "util/StringBuffer.h"
#include "util/String.h"
#include "util/PrintToString.h"
#include "key/key.h"
#include "backpacks/wifi/WifiModule.h"
extern "C" {
#include <js0n.h>
#include <j0g.h>
#include "lwm/sys/sysTimer.h"
}

#define container_of(ptr, type, member) ({ \
                const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                (type *)( (char *)__mptr - offsetof(type,member) );})

using namespace pinoccio;

static StringBuffer fieldCommand(0, 16);
static int fieldAnswerTo = 0;
static char *fieldAnswerChunks;
static int fieldAnswerChunksAt;
static int fieldAnswerRetries;
static NWK_DataReq_t fieldAnswerReq;

StringBuffer fieldCommandOutput;

// queue for mesh announcements
#define announceQsize 10
static char *announceQ[announceQsize];
static uint16_t announceQG[announceQsize];
static NWK_DataReq_t announceReq;
void announceQSend(void);

// mesh callback for handling incoming commands
static bool fieldCommands(NWK_DataInd_t *ind);

// chunk packet confirmation callback by mesh
static void fieldAnswerChunkConfirm(NWK_DataReq_t *req);

// send the first/next chunk of the answer back and confirm
static void fieldAnswerChunk();

// mesh callback whenever another scout announces something on a channel
static bool fieldAnnouncements(NWK_DataInd_t *ind);

// simple wrapper for the incoming channel announcements up to HQ
static void leadAnnouncementSend(uint16_t chan, uint16_t from, const ConstBuf& message);

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
static bool leadSignal(const String& json);

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind);


ScoutHandler::ScoutHandler() { }

ScoutHandler::~ScoutHandler() { }

void ScoutHandler::setup() {
  isBridged = false;
  if (Scout.isLeadScout()) {
    Scout.meshListen(3, leadAnswers);
    Scout.meshJoinGroup(0xBEEF); // our internal reporting channel
    Scout.meshJoinGroup(0); // reports to HQ
  }

  // join a set of groups to listen for announcements
  for (int i = 1; i < 10; i++) {
    Scout.meshJoinGroup(i);
  }

  // anyone is commandable (TODO refactor this w/ the command stuff now in Shell)
  Scout.meshListen(2, fieldCommands);
  Scout.meshListen(4, fieldAnnouncements);
  
  memset(announceQ,0,announceQsize*sizeof(char*));
}

void ScoutHandler::loop() {
  if (Scout.isLeadScout()) {
    leadHQHandle();
    // when the seen (most recent wifi read/write activity) is idle for 5+ minutes, paranoid reassociate
    if(SleepHandler::uptime().seconds - seen > 5*60)
    {
      if (isVerbose) {
        Serial.print(SleepHandler::uptime().seconds - seen);
        Serial.println(F(" seconds since last HQ activity, reconnecting"));
      }
      // TODO use a different flag to track this instead of overloading seen
      seen = SleepHandler::uptime().seconds - 60;
      WifiModule::instance.bp() && WifiModule::instance.bp()->associate();
    }
  }
}

void ScoutHandler::setBridged(bool flag) {
  isBridged = flag;
  if(isBridged)
  {
    bridge = "";
    Scout.meshListen(3, leadAnswers);
    Scout.meshJoinGroup(0xBEEF); // our internal reporting channel
    Scout.meshJoinGroup(0); // reports to HQ
    leadHQConnect();
  }
}

void ScoutHandler::setVerbose(bool flag) {
  isVerbose = flag;
}

static bool fieldCommands(NWK_DataInd_t *ind) {
  numvar ret;
  if (Scout.handler.isVerbose) {
    Serial.print(F("Received command"));
    Serial.print(F("lqi: "));
    Serial.print(ind->lqi);
    Serial.print(F("  "));
    Serial.print(F("rssi: "));
    Serial.println(ind->rssi);
  }

  if (fieldAnswerTo) {
    if (Scout.handler.isVerbose) {
      Serial.println(F("can't receive command while sending answer"));
    }
    return false;
  }

  if (!fieldCommand.concat((const char*)ind->data, ind->size)) {
    return false; // TODO we need to restart or something, no memory
  }

  // ack w/ our rssi as control
  NWK_SetAckControl(abs(ind->rssi));

  // when null terminated, do the message
  if (fieldCommand[fieldCommand.length() - 1] != '\0') {
    if (Scout.handler.isVerbose) {
      Serial.println(F("waiting for more"));
    }
    return true;
  }

  if (Scout.handler.isVerbose) {
    Serial.print(F("running command "));
    Serial.println(fieldCommand.c_str());
  }

  Shell.lastMeshRssi = abs(ind->rssi);
  Shell.lastMeshLqi = ind->lqi;
  Shell.lastMeshFrom = ind->srcAddr;

  fieldCommandOutput = "";
  ret = Shell.eval(PrintToString(fieldCommandOutput), fieldCommand);
  fieldCommand = (char*)NULL;

  if (Scout.handler.isVerbose) {
    Serial.print(F("got result "));
    Serial.println(ret);
    Serial.println(fieldCommandOutput.c_str());
  }
  
  // a command from the command endpoint doesn't get a response
  if(ind->srcEndpoint == 2)
  {
    fieldCommandOutput = (char*)NULL;
    return true;
  }

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
  fieldAnswerChunksAt = 0;
  fieldAnswerRetries = 0;
  fieldAnswerChunk();

  return true;
}

static void fieldAnswerChunkConfirm(NWK_DataReq_t *req) {
  if (Scout.handler.isVerbose) {
    Serial.print(F("  Message confirmation - "));
  }
  if (req->status == NWK_SUCCESS_STATUS) {
    if (Scout.handler.isVerbose) {
      Serial.println(F("success"));
    }
    if (fieldCommandOutput.length() - fieldAnswerChunksAt > 100) {
      fieldAnswerChunksAt += 100;
      fieldAnswerChunk();
      return; // don't free yet
    }
  } else {
    fieldAnswerRetries++;
    if (fieldAnswerRetries > 3) {
      if (Scout.handler.isVerbose) {
        Serial.print(F("error: "));
        Serial.println(req->status);
      }
    } else {
      if (Scout.handler.isVerbose) {
        Serial.println(F("RETRY"));
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

  if (Scout.handler.isVerbose) {
    Serial.print(fieldAnswerTo);
    Serial.print(F(" len "));
    Serial.print(len);
    Serial.println(F("->chunk"));
  }
}

static void announceConfirm(NWK_DataReq_t *req) {
  if (req->status != NWK_SUCCESS_STATUS && Scout.handler.isVerbose) {
    Serial.print(F("Mesh announce failed: "));
    Serial.println(req->status);
  }
  free(req->data);
  // slide queue over
  memmove(announceQ,announceQ+1,sizeof (char*)*(announceQsize-1));
  memmove(announceQG,announceQG+1,sizeof (uint16_t)*(announceQsize-1));
  announceQ[announceQsize-1] = 0;
  announceQSend();
}

void ScoutHandler::announce(uint16_t group, const String& message) {
  // when lead scout, share
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(group, Scout.getAddress(), message);
  }

  if (Scout.handler.isVerbose) {
    // TODO: This writes to Serial directly, but if we use the bitlash
    // sp functions while we're called from inside a command, this debug
    // output is added to the  command output, which isn't quite what we
    // want. There should be a better way to emit this kind of "log"
    // message.
    Serial.print(F("mesh announcing to "));
    Serial.print(group);
    Serial.print(F(" "));
    Serial.println(message);
  }

  // TODO: Allocate the data and request pointers in a single malloc?
  char *data = strdup(message.c_str());
  if (!data) {
    return;
  }

  if(announceQ[0])
  {
    int i = 1;
    while(i < announceQsize && announceQ[i]) i++;
    if(i == announceQsize) return (void)free(data);
    announceQ[i] = data;
    announceQG[i] = group;
    return;
  }
  announceQ[0] = data;
  announceQG[0] = group;
  announceQSend();
}

void announceQSend(void){
  if(!announceQ[0]) return;
  Scout.meshJoinGroup(announceQG[0]);
  announceReq.dstAddr = announceQG[0];
  announceReq.dstEndpoint = 4;
  announceReq.srcEndpoint = Scout.getAddress();
  announceReq.options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  announceReq.data = (uint8_t*)announceQ[0];
  announceReq.size = strlen(announceQ[0]) + 1; // include NUL byte
  announceReq.confirm = announceConfirm;
  NWK_DataReq(&announceReq);
}

static bool fieldAnnouncements(NWK_DataInd_t *ind) {
  char *data = (char*)ind->data;
  // be safe
  if (!ind->options & NWK_IND_OPT_MULTICAST) {
    return true;
  }

  if (Scout.handler.isVerbose) {
    Serial.print(F("multicast in "));
    Serial.println(ind->dstAddr);
  }
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(ind->dstAddr, ind->srcAddr, ConstBuf(data, ind->size-1)); // no null
  }
  if (!ind->dstAddr || ind->dstAddr == 0xBEEF || strlen(data) < 3 || data[0] != '[') {
    return false;
  }

  int keys[10];
  keyLoad((char*)ind->data, keys, millis());

  // run the Bitlash callback function, if defined
  StringBuffer callback(20);
  callback.appendSprintf("on.message.group", ind->dstAddr);
  if (Shell.defined((char*)callback.c_str())) {
    StringBuffer buf(64, 16);
    buf.appendSprintf("on.message.group(%d,%d", ind->dstAddr, ind->srcAddr);
    for (int i=2; i<=keys[0]; i++) {
      buf.appendSprintf(",%d", keys[i]);
    }
    buf += ")";
    Shell.eval(const_cast<char*>(buf.c_str()));
  }

  return true;
}

// just store one converted report at a time
StringBuffer report2json(const ConstBuf& in) {
  char *keys, *vals;
  unsigned short ir[16], ik[32], iv[32], i;
  StringBuffer reportJson(100, 8);

  // copy cuz we edit it
  char *report = (char*)malloc(in.length());
  memcpy(report, in, in.length());
  report[in.length()] = 0; // in's buf isn't null terminated for some reason

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

  // newer seq added to end is just a number
  if(ir[6])
  {
    report[ir[6]+ir[7]] = 0;
    reportJson.appendSprintf(",\"at\":%s",report+ir[6]);
  }
  reportJson += "}";
  free(report);
  return reportJson;
}


static void leadAnnouncementSend(uint16_t group, uint16_t from, const ConstBuf& message) {
  StringBuffer report(100, 8);

  // reports are expected to be json objects
  if (group == 0xBEEF) {
    report.appendSprintf("{\"type\":\"report\",\"from\":%d,\"report\":%s}\n", from, (report2json(message)).c_str());
  } else if (group == 0) {
    report.appendSprintf("{\"type\":\"announce\",\"from\":%d,\"announce\":", from);
    report.concat(message, message.length());
    report += "}\n";
  } else {
    return;
  }
  if(leadSignal(report))
  {
// TODO reentrancy issues, temporarily disabled!
//    Shell.eval("command.others","hq.online",1);
  }
}

// [3,[0,1,2],[v,v,v],4]
StringBuffer ScoutHandler::report(StringBuffer &report) {
  report.setCharAt(report.length() - 1, ',');
  report.appendSprintf("%lu]",millis());
  Scout.handler.announce(0xBEEF, report);
  return report2json(report);
}

////////////////////
// lead scout stuff

void leadHQConnect() {

  if (Scout.handler.isBridged || (WifiModule::instance.bp() && WifiModule::instance.bp()->client.connected())) {
    char token[33];
    StringBuffer auth(64);
    token[32] = 0;
    Scout.getHQToken(token);
    auth.appendSprintf("{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    leadSignal(auth);
  } else {
    if (Scout.handler.isVerbose) {
      Serial.println(F("server unvailable"));
    }
  }
}

// this is called on the main loop to process incoming data from HQ
static StringBuffer hqIncoming;
void leadHQHandle(void) {
  int rsize = 0;
  int nl;
  unsigned short index[32]; // <10 keypairs in the incoming json

  if(Scout.handler.isBridged)
  {
    rsize = (int)Scout.handler.bridge.length();
    hqIncoming += Scout.handler.bridge;
    Scout.handler.bridge = "";
  } else if (WifiModule::instance.bp()) {
    if (WifiModule::instance.bp()->client.available()) {
      rsize = hqIncoming.readClient(WifiModule::instance.bp()->client, 128);
      if(rsize > 0) Scout.handler.seen = SleepHandler::uptime().seconds;
    }
  }

  // only continue if new data to process
  if(rsize <= 0) return;
  
  // Read a block of data and look for packets
  while((nl = hqIncoming.indexOf('\n')) >= 0) {
    // look for a packet
    if(nl)
    {
      if (Scout.handler.isVerbose) {
        Serial.print(F("looking for packet in: "));
        Serial.println(hqIncoming);
      }

      // Parse JSON up to the first newline
      if (!js0n((const unsigned char*)hqIncoming.c_str(), nl, index, 32)) {
        leadIncoming(hqIncoming.c_str(), nl, index);
      } else {
        if (Scout.handler.isVerbose) {
          Serial.println(F("JSON parse failed"));
        }
      }
    }else{
      if (Scout.handler.isVerbose) {
        Serial.println(F("HQ ack'd"));
      }
    }

    // Remove up to and including the newline
    hqIncoming.remove(0, nl + 1);
  }
}

// when we can't process a command for some internal reason
void leadCommandError(int from, int id, const char *reason) {
  StringBuffer err(128);
  err.appendSprintf("{\"type\":\"reply\",\"from\":%d,\"id\":%d,\"err\":true,\"reply\":\"%s\"}\n", from, id, reason);
  leadSignal(err);
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
  if (Scout.handler.isVerbose) {
    Serial.println(type);
  }

  if (strcmp(type, "online") == 0) {
    // fire the online event for everyone
//    Shell.eval(F("command.all(\"hq.online\",2)"));
  }

  if (strcmp(type, "command") == 0) {
    to = atoi(j0g_str("to", buffer, index));
    id = strtoul(j0g_str("id", buffer, index), NULL, 10);
    command = j0g_str("command", buffer, index);
    if (strlen(j0g_str("to", buffer, index)) == 0 || !id || !command) {
      if (Scout.handler.isVerbose) {
        Serial.println(F("invalid command, requires to, id, command"));
        Serial.print(F("to: "));
        Serial.println(to);
        Serial.print(F("id: "));
        Serial.println(id);
        Serial.print(F("command: "));
        Serial.println(command);
      }
      free(buffer);
      return;
    }

    // handle internal ones first
    if (to == Scout.getAddress()) {
      Shell.eval(PrintToString(leadCommandOutput), command);

      StringBuffer report;
      report.appendSprintf("{\"type\":\"reply\",\"from\":%d,\"id\":%lu,\"end\":true,\"reply\":", to, id);
      report.appendJsonString(leadCommandOutput, true);
      report += "}\n";
      leadSignal(report);
      leadCommandOutput = (char*)NULL;

      free(buffer);
      return;
    }

    // we can only send one command at a time
    if (leadCommandTo) {
      // TODO we could stop reading the HQ socket in this mode and then never get a busy?
      free(buffer);
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

  free(buffer);
}

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req) {
  if (Scout.handler.isVerbose) {
    Serial.print(F("  Message confirmation - "));
  }
  if (req->status == NWK_SUCCESS_STATUS) {
    if (Scout.handler.isVerbose) {
      Serial.println(F("success"));
    }
    if (leadCommandChunks.length() - leadCommandChunksAt > 100) {
      leadCommandChunksAt += 100;
      leadCommandChunk();
      return; // don't free yet
    }
  } else {
    leadCommandRetries++;
    if (leadCommandRetries > 3) {
      if (Scout.handler.isVerbose) {
        Serial.print(F("error: "));
        Serial.println(req->status);
      }
      leadCommandError(leadCommandTo, leadAnswerID, "no response");
    } else {
      if (Scout.handler.isVerbose) {
        Serial.println(F("RETRY"));
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

  if (Scout.handler.isVerbose) {
    Serial.print(leadCommandTo);
    Serial.print(F(" len "));
    Serial.print(len);
    Serial.println(F("->chunk"));
  }
}

// wrapper to send a chunk of JSON to the HQ
bool leadSignal(const String &json) {
  if (Scout.handler.isBridged) {
    int i = 0;
    Shell.print("[hq-bridge] ");
    Shell.print(json.c_str());
    return true;
  }

  if (!WifiModule::instance.bp() || !WifiModule::instance.bp()->client.connected()) {
    if (Scout.handler.isVerbose) {
      Serial.println(F("HQ offline, can't signal"));
      Serial.println(json);
    }
    return false;
  }
  if (Scout.handler.isVerbose) {
    Serial.println(F("Signalling HQ: "));
    Serial.println(json);
  }

  WifiModule::instance.bp()->client.print(json);
  WifiModule::instance.bp()->client.flush();
  return true;
}

// called whenever another scout sends an answer back to us
bool leadAnswers(NWK_DataInd_t *ind) {
  bool end = false;
  StringBuffer buf(256);

  if (ind->options & NWK_IND_OPT_MULTICAST) {
    if (Scout.handler.isVerbose) {
      Serial.println(F("MULTICAST on wrong endpoint"));
    }
    return true;
  }

  if (Scout.handler.isVerbose) {
    Serial.print(F("Received answer from Scout "));
    Serial.print(ind->srcAddr);
    Serial.println(F(":"));
  }
  if (ind->data[ind->size-1] == 0) {
    end = true;
    ind->size--;
  }
  buf.appendSprintf("{\"type\":\"reply\",\"id\":%d,\"from\":%d,\"reply\":", leadAnswerID, ind->srcAddr);
  buf.appendJsonString(ind->data, ind->size, true);
  buf.appendSprintf(",\"end\":%s}\n",end ? "true" : "false");
  leadSignal(buf);

  return true;
}
