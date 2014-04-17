
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
extern "C" {
#define DEBUG 1
#define CS_1a 1
#include <avr.h>
#include <switch.h>
  void println(char *x) { Serial.println(x); }
}

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

  DEBUG_PRINTF("looking for packets to send");
  while((p = switch_sending(ths)))
  {
    if(util_cmp(p->out->type,"ipv4")!=0)
    {
      packet_free(p);
      continue;
    }
    DEBUG_PRINTF("sending %s packet %d %s\n",p->json_len?"open":"line",packet_len(p),path_json(p->out));
    writePacket(path_ip(p->out,0),path_port(p->out,0),packet_raw(p),packet_len(p));
    packet_free(p);
  }

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

void setup() {
  Scout.setup();  
  Scout.wifi.onOn = onOn;
  platform_debugging(1);
  
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
  DEBUG_PRINTF("loaded seed %s",seed->hexname);
  
}

void loop() {
  Scout.loop();
  readPacket();
}
