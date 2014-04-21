
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
#include <avr.h>
void println(char *x) { Serial.println(x); }
}
void localWrite(packet_t p);

switch_t ths;
hn_t seed;
uint8_t isOn = 0;

void writePacket(char *ip, uint16_t port, unsigned char *msg, int len)
{
  DEBUG_PRINTF("writing %s:%hu %d",ip,port,len);
  Scout.wifi.server.beginPacket(ip,port);
  Scout.wifi.server.write(msg,len);
  Scout.wifi.server.endPacket();
}

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

void readPacket()
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

unsigned char *localBuf;
int localBufLen = 0;
static numvar localIn(void)
{
  if(getarg(1) && isstringarg(1))
  {
    const char *str = (const char *)getarg(1);
    int len = strlen(str)/2;
    // buffering up mode
    if(len)
    {
      unsigned char *buf = (unsigned char*)realloc(localBuf,localBufLen+len);
      if(!buf)
      {
        free(localBuf);
        speol("out of memory ");
        speol(localBufLen+len);
        localBufLen = 0;
        return 0;
      }
      localBuf = buf;
      util_unhex((unsigned char*)str,len*2,localBuf+localBufLen);
      localBufLen += len;
      sp("buffered ");
      speol(localBufLen);
    }
    return 1;
  }
  if(!localBufLen) return 0;
  // packet parsing
  path_t from = path_new("local");
  packet_t p = packet_parse(localBuf,localBufLen);
  free(localBuf);
  localBufLen = 0;
  if(!p)
  {
    speol("invalid packet");
    return 0;
  }
  speol("processing");
  DEBUG_PRINTF("received %s packet %d", p->json_len?(p->json_len>1?packet_get_str(p,"type"):"open"):"line", packet_len(p));
  switch_receive(ths,p,from);
  return 1;
}

void localWrite(packet_t p)
{
  unsigned char *hex = (unsigned char *)malloc(packet_len(p)*2+1);
  if(!hex)
  {
    sp("no memory ");
    speol(packet_len(p)*2+1);
  }else{
    sp("\ntelehash:");
    speol((char*)util_hex(packet_raw(p),packet_len(p),hex));    
    free(hex);
  }
  packet_free(p);
}

// check for outgoing packets
void sendLoop(void)
{
  packet_t p;
  while((p = switch_sending(ths)))
  {
    DEBUG_PRINTF("sending %s %s packet %d %s",p->out->type,p->json_len?"open":"line",packet_len(p),path_json(p->out));
    if(util_cmp(p->out->type,"ipv4") == 0 && isOn)
    {
      writePacket(path_ip(p->out,0),path_port(p->out,0),packet_raw(p),packet_len(p));
      packet_free(p);
    }else if(util_cmp(p->out->type,"local") == 0){
       localWrite(p);
    }else{
      DEBUG_PRINTF("%s packet %hu no network for %s",p->json_len?"open":"line",packet_len(p),path_json(p->out));
      packet_free(p);
    }
  }
}

void setup() {
  Scout.setup();  
  Scout.wifi.onOn = onOn;
  platform_debugging(1);
  addBitlashFunction("telehash", (bitlash_function) localIn);
  crypt_init();
  
  char keyjs[] = "{\"1a\":\"kNcFp/4mRAS7lSHjhfCfAHZv264e8nGONBuenfzJS3gSCJmHDbpy4w==\",\"1a_secret\":\"estHaS+O5xRYGyPhLcXL9Iaggfk=\"}";
  packet_t keys = packet_new();
//  crypt_keygen(0x1a,keys);
  packet_json(keys,(unsigned char*)keyjs,strlen(keyjs));

  char seedjs[] = "{\"paths\":[{\"type\":\"ipv4\",\"ip\":\"192.168.0.36\",\"port\":42424}],\"parts\":{\"1a\":\"821e083c2b788c75bf4608e66a52ef2d911590f6\"},\"keys\":{\"1a\":\"z6yCAC7r5XIr6C4xdxeX7RlSmGu9Xe73L1gv8qecm4/UEZAKR5iCxA==\"}}";
  ths = switch_new(41);
  if(switch_init(ths,keys)) Serial.println("switch init failed");
  DEBUG_PRINTF("loaded hashname %s",ths->id->hexname);
  packet_t p = packet_new();
  packet_json(p,(unsigned char*)seedjs,strlen(seedjs));
  seed = hn_fromjson(ths->index,p);
  packet_free(p);
  DEBUG_PRINTF("loaded seed %s",seed->hexname);
  
  localWrite(switch_ping(ths));
}

void loop() {
  Scout.loop();
  readPacket();
  sendLoop();
}
