
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
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
  if(!Scout.wifi.server.begin(42424))
    Serial.println("Bind failed");

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
  packet_t p = packet_parse(buf,len);
  free(buf);
  printf("received %s packet %d %s\n", p->json_len?"open":"line", len, path_json(from));
  switch_receive(ths,p,from);
}

static numvar localIn(void)
{
  if(!getarg(1) || !isstringarg(1))
  {
    speol("missing arg");
    return 0;
  }
  const char *str = (const char *)getarg(1);
  int len = strlen(str)/2;
  unsigned char *raw = (unsigned char *)malloc(len);
  util_unhex((unsigned char*)str,len*2,raw);
  path_t from = path_new("local");
  packet_t p = packet_parse(raw,len);
  free(raw);
  if(!p)
  {
    speol("invalid packet");
    return 0;
  }
  printf("received %s packet %d\n", p->json_len?"open":"line", len);
  switch_receive(ths,p,from);
  return 1;
}

void localWrite(packet_t p)
{
  sp("telehash:");
  unsigned char *hex = (unsigned char *)malloc(packet_len(p)*2+1);
  speol((char*)util_hex(packet_raw(p),packet_len(p),hex));
  free(hex);
}

// check for outgoing packets
void sendLoop(void)
{
  packet_t p;
  while((p = switch_sending(ths)))
  {
    if(util_cmp(p->out->type,"ipv4")==0 && isOn)
    {
      DEBUG_PRINTF("sending ipv4 %s packet %d %s\n",p->json_len?"open":"line",packet_len(p),path_json(p->out));
      writePacket(path_ip(p->out,0),path_port(p->out,0),packet_raw(p),packet_len(p));
    }
    if(util_cmp(p->out->type,"local")==0) localWrite(p);
    packet_free(p);
  }
}

void setup() {
  Scout.setup();  
  Scout.wifi.onOn = onOn;
  platform_debugging(1);
  addBitlashFunction("telehash", (bitlash_function) localIn);
  
  char seedjs[] = "{\"paths\":[{\"type\":\"ipv4\",\"ip\":\"192.168.0.36\",\"port\":42424}],\"parts\":{\"1a\":\"821e083c2b788c75bf4608e66a52ef2d911590f6\"},\"keys\":{\"1a\":\"z6yCAC7r5XIr6C4xdxeX7RlSmGu9Xe73L1gv8qecm4/UEZAKR5iCxA==\"}}";
  packet_t keys = packet_new();
  crypt_init();
  crypt_keygen(0x1a,keys);
  ths = switch_new(41);
  if(switch_init(ths,keys)) Serial.println("switch init failed");
  DEBUG_PRINTF("loaded hashname %s",ths->id->hexname);
  packet_t p = packet_new();
  packet_json(p,(unsigned char*)seedjs,strlen(seedjs));
  seed = hn_fromjson(ths->index,p);
  packet_free(p);
  DEBUG_PRINTF("loaded seed %s",seed->hexname);
  
  p = packet_new();
  packet_set_str(p,"type","ping");
  packet_set(p,"1a","true",4);
  localWrite(p);
  packet_free(p);
}

void loop() {
  Scout.loop();
  readPacket();
}
