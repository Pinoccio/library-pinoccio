
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
#include <AESLib.h>
extern "C" {
#include <switch.h>
#include <ext.h>
#include <avr.h>
void println(char *x) { Serial.println(x); Serial.flush(); }
}
void writeSerial(packet_t p);

switch_t ths;
hn_t seed;
uint8_t isOn = 0;

void onOn()
{
  IPAddress ip;
  Serial.println("online");
  if(!Scout.wifi.server.begin(42424)) {
    Serial.println("Bind failed");
    // this probably just means that we're already bound, since onOn is
    // also called after the TCP connection reconnects, without
    // reassociating
    return;
  }

  isOn = true;
  // create/send a ping packet  
  chan_t c = chan_new(ths, seed, "seek", 0);
  packet_t p = chan_packet(c);
  packet_set_str(p,"seek",ths->id->hexname);
  DEBUG_PRINTF("seek %d",p->json_len);
  chan_send(c, p);

}

void readIPv4()
{
  if(!isOn) return;
  unsigned char *buf;
  // read a single UDP packet
  size_t len = Scout.wifi.server.parsePacket();
  if(!len) return;
  buf = (unsigned char*)malloc(len);

  path_t from = path_new("ipv4");
  path_ip4(from,Scout.wifi.server.remoteIP());
  path_port(from,Scout.wifi.server.remotePort());
  size_t read = 0;
  while(read < len)
    read += Scout.wifi.server.read(buf + read, len - read);

  DEBUG_PRINTF("received packet %d %s\n", len, path_json(from));
  printHexBuffer(Serial, buf, len);

  packet_t p = packet_parse(buf,len);
  if (!p) {
    DEBUG_PRINTF("Failed to parse packet");
    return;
  }
  
  free(buf);
  switch_receive(ths,p,from);
}

void writeIPv4(char *ip, uint16_t port, unsigned char *msg, int len)
{
  if(!isOn) return;
  DEBUG_PRINTF("writing %s:%hu %d",ip,port,len);
  Scout.wifi.server.beginPacket(ip,port);
  Scout.wifi.server.write(msg,len);
  Scout.wifi.server.endPacket();
}

void writeSerial(packet_t p)
{
  unsigned char blen, *raw = packet_raw(p);
  int len = packet_len(p);

  blen = (char)((len-1)/256);
  Serial.write('\n');
  Serial.write((char)42);
  Serial.write(blen);
  Serial.write((char)(len - (blen*256)));
  Serial.write(raw,len);
  Serial.flush();
  packet_free(p);
}

void serialParse(StringBuffer *buf)
{
  packet_t p = packet_parse((unsigned char*)buf->c_str(),buf->length());
  if(!p)
  {
    DEBUG_PRINTF("serial packet parse failed len %d %s",buf->length());
    return;
  }
  RgbLed.blinkGreen(100);
  path_t from = path_new("local");
  DEBUG_PRINTF("received %s packet %d", p->json_len?(p->json_len>1?packet_get_str(p,"type"):"open"):"line", packet_len(p));
  switch_receive(ths,p,from);
}

// serial read buffering/tracking
StringBuffer bufS(100);
char modeS = 0;
uint8_t blockS, readS;
uint16_t countS = 0;
void readSerial(void)
{
  char c, more = 0;
  
//  if(Serial.available()) DEBUG_PRINTF("read %d %d %d %d %d %d",Serial.available(),modeS,countS,blockS,readS,bufS.length());
  while(Serial.available() > 0)
  {
    c = (char)Serial.read();
    countS++;
    switch(modeS) {
      case 0: // first char on new line
//      DEBUG_PRINTF("zero %d",c);
        // detect read packet start
        if(c == 42)
        {
          modeS = 2;
          continue;
        }
        // fall through
        modeS = 1;
      case 1: // normal serial commands
        Serial.write(c); // echo
        bufS += c;
        if(c != '\n') continue;
        if(bufS.length() > 1) doCommand((char*)bufS.c_str());
        Serial.write("> ",2);
        bufS = "";
        modeS = 0;
        break;
      case 2: // read block len
        blockS = (uint8_t)c;
        readS = 0;
        modeS = 3;
//        DEBUG_PRINTF("block read %d %d",blockS,bufS.length());
        if(blockS) continue;
        // empty block means packet received
        serialParse(&bufS);
        bufS = "";
        modeS = 0;
        break;
      case 3: // reading block
        bufS += c;
        readS++;
        if(readS < blockS) continue;
//        DEBUG_PRINTF("block done %d %d",blockS,bufS.length());
        modeS = 2;
        // sending empty packet signals ready for another block
        writeSerial(NULL);
    }
  }
}

// look for any incoming packets
void readLoop(void)
{
  readIPv4();
  readSerial();
}

// check for outgoing packets
void writeLoop(void)
{
  packet_t p;
  while((p = switch_sending(ths)))
  {
    RgbLed.blinkBlue(100);
    DEBUG_PRINTF("sending %s %s packet %d %s",p->out->type,p->json_len?"open":"line",packet_len(p),path_json(p->out));
    if(util_cmp(p->out->type,"ipv4") == 0 && isOn)
    {
      writeIPv4(path_ip(p->out,0),path_port(p->out,0),packet_raw(p),packet_len(p));
      packet_free(p);
    }else if(util_cmp(p->out->type,"local") == 0){
      writeSerial(p);
    }else{
      DEBUG_PRINTF("%s packet %hu no network for %s",p->json_len?"open":"line",packet_len(p),path_json(p->out));
      packet_free(p);
    }
  }
}

void strangeLoop(void)
{
  chan_t c;
  packet_t p;

  // any incoming active channels
  while((c = switch_pop(ths)))
  {
    DEBUG_PRINTF("channel active %d %s %s\n",c->state,c->hexid,c->to->hexname);
    if(util_cmp(c->type,"connect") == 0) ext_connect(c);
    if(util_cmp(c->type,"link") == 0) ext_link(c);
    if(util_cmp(c->type,"path") == 0) ext_path(c);
    while((p = chan_pop(c)))
    {
      DEBUG_PRINTF("unhandled channel packet %.*s\n", p->json_len, p->json);      
      packet_free(p);
    }
    if(c->state == CHAN_ENDED) chan_free(c);
  }

}

void setup() {
  Scout.setup();  
  Scout.wifi.onOnline = onOn;
  platform_debugging(1);
  crypt_init();
  
  char keyjs[] = "{\"1a\":\"kNcFp/4mRAS7lSHjhfCfAHZv264e8nGONBuenfzJS3gSCJmHDbpy4w==\",\"1a_secret\":\"estHaS+O5xRYGyPhLcXL9Iaggfk=\"}";
  packet_t keys = packet_new();
//  crypt_keygen(0x1a,keys);
  packet_json(keys,(unsigned char*)keyjs,strlen(keyjs));

  char seedjs[] = "{\"paths\":[{\"type\":\"ipv4\",\"ip\":\"192.168.0.36\",\"port\":42424}],\"parts\":{\"1a\":\"b5a96d25802b3600ea99774138a650d5d1fa1f3cf3cb10ae8f1c58a527d85086\"},\"keys\":{\"1a\":\"z6yCAC7r5XIr6C4xdxeX7RlSmGu9Xe73L1gv8qecm4/UEZAKR5iCxA==\"}}";
  ths = switch_new(41);
  if(switch_init(ths,keys)) Serial.println("switch init failed");
  DEBUG_PRINTF("loaded hashname %s",ths->id->hexname);
  packet_t p = packet_new();
  if(packet_json(p,(unsigned char*)seedjs,strlen(seedjs))) Serial.println("seed json parse failed");
  seed = hn_fromjson(ths->index,p);
  packet_free(p);
  DEBUG_PRINTF("loaded seed %s",seed->hexname);
}

long lastt = millis();
void loop() {
  readLoop();
  writeLoop();
  strangeLoop();
  Scout.loop();
  if(millis() - lastt > 5*1000)
  {
    lastt = millis();
    DEBUG_PRINTF("tick %d %d %d %d %d",modeS,countS,blockS,bufS.length(),lastt);
  }
}
