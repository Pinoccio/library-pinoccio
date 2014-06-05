/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include "HqHandler.h"
#include <avr/pgmspace.h>
#include <Shell.h>
#include <Scout.h>
#include "../backpack-bus/PBBP.h"
#include "../backpacks/wifi/WiFiBackpack.h"
#include "../util/String.h"
#include "../util/PrintToString.h"
extern "C" {
#include <js0n.h>
#include <j0g.h>
#include "../key/key.h"
#include "lwm/sys/sysTimer.h"
}

// created using:
// json2c.js: console.log("{'"+JSON.stringify(require(require("path").resolve(process.argv[2]))).split("").join("','")+"'};");
// node json2c.js seeds.json 

static const char seeds_json[] PROGMEM = {'{','"','d','c','a','5','4','9','c','9','8','b','9','4','1','9','7','e','7','9','d','f','a','2','a','7','e','4','a','d','2','e','7','4','f','b','1','4','4','c','d','a','3','f','4','7','1','0','d','4','f','4','0','e','2','c','7','5','d','9','7','5','2','7','2','e','"',':','{','"','p','a','t','h','s','"',':','[','{','"','t','y','p','e','"',':','"','h','t','t','p','"',',','"','h','t','t','p','"',':','"','h','t','t','p',':','/','/','1','9','2','.','1','6','8','.','0','.','3','6',':','4','2','4','2','4','"','}',',','{','"','t','y','p','e','"',':','"','i','p','v','4','"',',','"','i','p','"',':','"','1','2','7','.','0','.','0','.','1','"',',','"','p','o','r','t','"',':','4','2','4','2','4','}',',','{','"','t','y','p','e','"',':','"','i','p','v','6','"',',','"','i','p','"',':','"','f','e','8','0',':',':','b','a','e','8',':','5','6','f','f',':','f','e','4','3',':','3','d','e','4','"',',','"','p','o','r','t','"',':','4','2','4','2','4','}',']',',','"','p','a','r','t','s','"',':','{','"','3','a','"',':','"','f','0','d','2','b','f','c','8','5','9','0','a','7','e','0','0','1','6','c','e','8','5','d','b','f','0','f','8','f','1','8','8','3','f','b','4','f','3','d','c','c','4','7','0','1','e','a','b','1','2','e','f','8','3','f','9','7','2','a','2','b','8','7','f','"',',','"','2','a','"',':','"','0','c','b','4','f','6','1','3','7','a','7','4','5','f','1','a','f','2','d','3','1','7','0','7','5','5','0','c','0','3','b','9','9','0','8','3','1','8','0','f','6','e','6','9','e','c','3','7','9','1','8','c','2','2','0','e','c','f','a','2','9','7','2','f','"',',','"','1','a','"',':','"','b','5','a','9','6','d','2','5','8','0','2','b','3','6','0','0','e','a','9','9','7','7','4','1','3','8','a','6','5','0','d','5','d','1','f','a','1','f','3','c','f','3','c','b','1','0','a','e','8','f','1','c','5','8','a','5','2','7','d','8','5','0','8','6','"','}',',','"','k','e','y','s','"',':','{','"','3','a','"',':','"','M','C','5','d','f','S','f','r','A','V','C','S','u','g','X','7','5','J','b','g','V','W','t','v','C','b','x','P','q','w','L','D','U','k','c','9','T','c','S','/','q','x','E','=','"',',','"','2','a','"',':','"','M','I','I','B','I','j','A','N','B','g','k','q','h','k','i','G','9','w','0','B','A','Q','E','F','A','A','O','C','A','Q','8','A','M','I','I','B','C','g','K','C','A','Q','E','A','q','r','1','2','t','X','n','p','n','7','0','7','l','l','k','Z','f','E','c','s','p','B','/','D','6','K','T','c','Z','M','7','6','5','+','S','n','I','5','Z','8','J','W','k','j','c','0','M','r','z','9','q','Z','B','B','2','Y','F','L','r','2','N','m','g','C','x','0','o','L','f','S','e','t','m','u','H','B','N','T','T','5','4','s','I','A','x','Q','/','v','x','y','y','k','c','M','N','G','s','S','F','g','4','W','K','h','b','s','Q','X','S','r','X','4','q','C','h','b','h','p','I','q','M','J','k','K','a','4','m','Y','Z','I','b','6','q','O','N','A','7','6','G','5','/','4','3','1','u','4','+','1','s','B','R','v','f','Y','0','e','w','H','C','h','q','G','h','0','o','T','h','c','a','a','5','0','n','T','6','8','f','8','o','h','I','s','1','i','U','F','m','+','S','L','8','L','9','U','L','/','o','K','N','3','Y','g','6','d','r','B','Y','w','p','J','i','2','E','x','5','I','d','y','u','4','Y','Q','J','w','Z','9','s','A','Q','U','4','9','P','f','s','+','L','q','h','k','H','O','a','s','c','T','m','a','a','3','+','k','T','y','T','n','p','2','i','J','9','w','E','u','P','g','+','A','R','3','P','J','w','x','X','n','w','Y','o','W','b','H','+','W','r','8','g','Y','6','i','L','e','0','F','Q','e','8','j','X','k','6','e','L','w','9','m','q','O','h','U','c','a','h','8','3','3','8','M','C','8','3','z','S','Q','c','Z','r','i','G','V','M','q','8','q','a','Q','z','0','L','9','n','w','I','D','A','Q','A','B','"',',','"','1','a','"',':','"','z','6','y','C','A','C','7','r','5','X','I','r','6','C','4','x','d','x','e','X','7','R','l','S','m','G','u','9','X','e','7','3','L','1','g','v','8','q','e','c','m','4','/','U','E','Z','A','K','R','5','i','C','x','A','=','=','"','}','}','}'};


static bool hqVerboseOutput;

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
static void leadSignal(const String& json);

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind);


HqHandler::HqHandler() { }

HqHandler::~HqHandler() { }

void HqHandler::setup() {

  Scout.meshListen(2, fieldCommands);
  Scout.meshListen(3, leadAnswers);
  Scout.meshListen(4, fieldAnnouncements);

  Scout.meshJoinGroup(0xBEEF); // our internal reporting channel
  Scout.meshJoinGroup(0); // reports to HQ

  // join a set of groups to listen for announcements
  for (int i = 1; i < 10; i++) {
    Scout.meshJoinGroup(i);
  }
  
  memset(announceQ,0,announceQsize*sizeof(char*));

}

bool HqHandler::connected() {
  return false;
}

bool HqHandler::available() {
  return false;
}

void HqHandler::loop() {
  leadHQHandle();
}

bool HqHandler::isBridge() {
  return false;
}

void HqHandler::up(UDP *out) {
  uout = out;
  // TODO send flush/ping packets now
}

void HqHandler::setVerbose(bool flag) {
  hqVerboseOutput = flag;
}

static bool fieldCommands(NWK_DataInd_t *ind) {
  int ret;
  if (hqVerboseOutput) {
    Serial.print(F("Received command"));
    Serial.print(F("lqi: "));
    Serial.print(ind->lqi);
    Serial.print(F("  "));
    Serial.print(F("rssi: "));
    Serial.println(ind->rssi);
  }

  if (fieldAnswerTo) {
    if (hqVerboseOutput) {
      Serial.println(F("can't receive command while sending answer"));
    }
    return false;
  }

  if (!fieldCommand.concat((const char*)ind->data, ind->size)) {
    return false; // TODO we need to restart or something, no memory
  }

  // when null terminated, do the message
  if (fieldCommand[fieldCommand.length() - 1] != '\0') {
    if (hqVerboseOutput) {
      Serial.println(F("waiting for more"));
    }
    return true;
  }

  if (hqVerboseOutput) {
    Serial.print(F("running command "));
    Serial.println(fieldCommand);
  }

  // run the command and chunk back the results
  setOutputHandler(&printToString<&fieldCommandOutput>);

  // TODO: Once bitlash fixes const-correctness, this and similar casts
  // should be removed.
  ret = (int)doCommand(const_cast<char *>(fieldCommand.c_str()));
  fieldCommand = (char*)NULL;

  resetOutputHandler();

  if (hqVerboseOutput) {
    Serial.print(F("got result "));
    Serial.println(ret);
  }

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
  fieldAnswerChunksAt = 0;
  fieldAnswerRetries = 0;
  fieldAnswerChunk();

  return true;
}

static void fieldAnswerChunkConfirm(NWK_DataReq_t *req) {
  if (hqVerboseOutput) {
    Serial.print(F("  Message confirmation - "));
  }
  if (req->status == NWK_SUCCESS_STATUS) {
    if (hqVerboseOutput) {
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
      if (hqVerboseOutput) {
        Serial.print(F("error: "));
        Serial.println(req->status);
      }
    } else {
      if (hqVerboseOutput) {
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

  if (hqVerboseOutput) {
    Serial.print(fieldAnswerTo);
    Serial.print(F(" len "));
    Serial.print(len);
    Serial.println(F("->chunk"));
  }
}

static void announceConfirm(NWK_DataReq_t *req) {
  if (req->status != NWK_SUCCESS_STATUS && hqVerboseOutput) {
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

void HqHandler::announce(uint16_t group, const String& message) {
  // when lead scout, shortcut
  if (Scout.isLeadScout()) {
    leadAnnouncementSend(group, Scout.getAddress(), message);
    // Don't broadcast HQ commands over the network if we are a lead
    // scout
    if (!group || group == 0xBEEF)
      return;
  }

  if (hqVerboseOutput) {
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

  if (hqVerboseOutput) {
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
  if (findscript(const_cast<char*>(callback.c_str()))) {
    StringBuffer buf(64, 16);
    buf.appendSprintf("on.message.group(%d,%d", ind->dstAddr, ind->srcAddr);
    for (int i=2; i<=keys[0]; i++) {
      buf.appendSprintf(",%d", keys[i]);
    }
    buf += ")";
    doCommand(const_cast<char*>(buf.c_str()));
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
  leadSignal(report);
}

// [3,[0,1,2],[v,v,v]]
StringBuffer HqHandler::report(const String &report) {
  Scout.hq.announce(0xBEEF, report);
  return report2json(report);
}

////////////////////
// lead scout stuff

void leadHQConnect() {

  if (Scout.hq.connected()) {
    char token[33];
    StringBuffer auth(64);
    token[32] = 0;
    Scout.getHQToken(token);
    auth.appendSprintf("{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    leadSignal(auth);
  } else {
    if (hqVerboseOutput) {
      Serial.println(F("server unvailable"));
    }
  }
}

// this is called on the main loop to process incoming data from HQ
static StringBuffer hqIncoming;
void leadHQHandle(void) {
  int rsize;
  unsigned short index[32]; // <10 keypairs in the incoming json

  // only continue if new data to read
  if (!Scout.hq.available()) {
    return;
  }

  /*
  // Read a block of data and look for packets
  while ((rsize = hqIncoming.readClient(Scout.wifi.client, 128))) {
    int nl;
    while((nl = hqIncoming.indexOf('\n')) >= 0) {
     // look for a packet
      if (hqVerboseOutput) {
        Serial.print(F("looking for packet in: "));
        Serial.println(hqIncoming);
      }

      // Parse JSON up to the first newline
      if (!js0n((const unsigned char*)hqIncoming.c_str(), nl, index, 32)) {
        leadIncoming(hqIncoming.c_str(), nl, index);
      } else {
        if (hqVerboseOutput) {
          Serial.println(F("JSON parse failed"));
        }
      }

      // Remove up to and including the newline
      hqIncoming.remove(0, nl + 1);
    }
  }
  */
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
  if (hqVerboseOutput) {
    Serial.println(type);
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
      setOutputHandler(&printToString<&leadCommandOutput>);
      doCommand(command);
      resetOutputHandler();

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
  if (hqVerboseOutput) {
    Serial.print(F("  Message confirmation - "));
  }
  if (req->status == NWK_SUCCESS_STATUS) {
    if (hqVerboseOutput) {
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
      if (hqVerboseOutput) {
        Serial.print(F("error: "));
        Serial.println(req->status);
      }
      leadCommandError(leadCommandTo, leadAnswerID, "no response");
    } else {
      if (hqVerboseOutput) {
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

  if (hqVerboseOutput) {
    Serial.print(leadCommandTo);
    Serial.print(F(" len "));
    Serial.print(len);
    Serial.println(F("->chunk"));
  }
}

// wrapper to send a chunk of JSON to the HQ
void leadSignal(const String &json) {
  if (!Scout.hq.connected()) {
    if (hqVerboseOutput) {
      Serial.println(F("HQ offline, can't signal"));
      Serial.println(json);
    }
    return;
  }
  if (hqVerboseOutput) {
    Serial.println(F("Signalling HQ: "));
    Serial.println(json);
  }

//  Scout.wifi.client.print(json);
//  Scout.wifi.client.flush();
}

// called whenever another scout sends an answer back to us
bool leadAnswers(NWK_DataInd_t *ind) {
  bool end = false;
  StringBuffer buf(256);

  if (ind->options & NWK_IND_OPT_MULTICAST) {
    if (hqVerboseOutput) {
      Serial.println(F("MULTICAST on wrong endpoint"));
    }
    return true;
  }

  if (hqVerboseOutput) {
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
