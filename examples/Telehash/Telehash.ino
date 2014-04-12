
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
  Scout.setup();  
  Scout.wifi.onOn = onOn;
  platform_debugging(1);
  packet_t keys = packet_new();
  crypt_init();
  crypt_keygen(0x1a,keys);
  DEBUG_PRINTF("keys %d %s",keys->json_len,keys->json);
  ths = switch_new();
  switch_init(ths,keys);
  Serial.println((char*)ths->id->hexname);
  DEBUG_PRINTF("loaded hashname %d %s",strlen(ths->id->hexname),ths->id->hexname);
  DEBUG_PRINTF("parts %d %s",ths->parts->json_len,ths->parts->json);
}

void loop() {
  Scout.loop();
  readPacket();
}
