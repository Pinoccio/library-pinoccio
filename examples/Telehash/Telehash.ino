
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
#include <ecc.h>
void println(char *x) { Serial.println(x); }
}

switch_t ths;

void onOn()
{
  Serial.println("online");
  if(!Scout.wifi.server.begin(42424))
    Serial.println("Bind failed");
}

void readPacket()
{
  // read a single UDP packet
  size_t len = Scout.wifi.server.parsePacket();
  if(!len) return;

  Serial.print("packet len ");
  Serial.print(len);
  Serial.print(" from ");
  Serial.print(Scout.wifi.server.remoteIP());
  Serial.print(":");
  Serial.print(Scout.wifi.server.remotePort());
  Serial.println();

  while (Scout.wifi.server.available()) {
    int c = Scout.wifi.server.read();
    Serial.write(c);
  }

  Scout.wifi.server.beginPacket(Scout.wifi.server.remoteIP(),Scout.wifi.server.remotePort());
  Scout.wifi.server.write("sup\n",4);
  Scout.wifi.server.endPacket();
  
}

void setup() {
  char seedjs[] = "{\"paths\":[{\"type\":\"ipv4\",\"ip\":\"192.168.0.36\",\"port\":42424}],\"parts\":{\"3a\":\"f0d2bfc8590a7e0016ce85dbf0f8f1883fb4f3dcc4701eab12ef83f972a2b87f\",\"2a\":\"0cb4f6137a745f1af2d31707550c03b99083180f6e69ec37918c220ecfa2972f\",\"1a\":\"821e083c2b788c75bf4608e66a52ef2d911590f6\"},\"keys\":{\"3a\":\"MC5dfSfrAVCSugX75JbgVWtvCbxPqwLDUkc9TcS/qxE=\",\"2a\":\"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqr12tXnpn707llkZfEcspB/D6KTcZM765+SnI5Z8JWkjc0Mrz9qZBB2YFLr2NmgCx0oLfSetmuHBNTT54sIAxQ/vxyykcMNGsSFg4WKhbsQXSrX4qChbhpIqMJkKa4mYZIb6qONA76G5/431u4+1sBRvfY0ewHChqGh0oThcaa50nT68f8ohIs1iUFm+SL8L9UL/oKN3Yg6drBYwpJi2Ex5Idyu4YQJwZ9sAQU49Pfs+LqhkHOascTmaa3+kTyTnp2iJ9wEuPg+AR3PJwxXnwYoWbH+Wr8gY6iLe0FQe8jXk6eLw9mqOhUcah8338MC83zSQcZriGVMq8qaQz0L9nwIDAQAB\",\"1a\":\"z6yCAC7r5XIr6C4xdxeX7RlSmGu9Xe73L1gv8qecm4/UEZAKR5iCxA==\"}}";
  Scout.setup();  
  Scout.wifi.onOn = onOn;
  platform_debugging(1);
  packet_t keys = packet_new();
  crypt_init();
  crypt_keygen(0x1a,keys);
  DEBUG_PRINTF("keys %d %s",keys->json_len,keys->json);
  ths = switch_new();
  if(switch_init(ths,keys)) Serial.println("switch init failed");
  Serial.println((char*)ths->id->hexname);
  DEBUG_PRINTF("loaded hashname %s",ths->id->hexname);
  packet_t p = packet_new();
  packet_json(p,(unsigned char*)seedjs,strlen(seedjs));
  hn_t seed = hn_fromjson(ths->index,p);
  DEBUG_PRINTF("loaded seed %s",seed->hexname);
}

void loop() {
  Scout.loop();
  readPacket();
}
