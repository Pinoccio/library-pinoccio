/*
 mqttClient.h - A simple client for MQTT.
  Nicholas O'Leary
  http://knolleary.net
*/

#ifndef _PINOCCIO_MQTTS_CLIENT_H
#define _PINOCCIO_MQTTS_CLIENT_H

#include <Arduino.h>
#include <Pinoccio.h>

// MQTT_MAX_PACKET_SIZE : Maximum packet size is NWK_MAX_SECURED_PAYLOAD_SIZE b/c of security headers
#define MQTTS_MAX_PACKET_SIZE 105 

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTTS_KEEPALIVE 300
#define MQTTSPROTOCOLVERSION 3

typedef enum {
  MQTTS_ADVERTISE     0x00,  // Gateway advertising its address
  MQTTS_SEARCHGW      0x01,  // Client requesting response for gateway
  MQTTS_SEARCHGW      0x02,  // 
  //RESERVED          0x03,  
  MQTTS_CONNECT       0x04,  // Client request to connect to Server
  MQTTS_CONNACK       0x05,  // Connect Acknowledgment
  MQTTS_WILLTOPICREQ  0x06,  // Connect Acknowledgment
  MQTTS_WILLTOPIC     0x07,  // Connect Acknowledgment
  MQTTS_WILLMSGREQ    0x08,  // Connect Acknowledgment
  MQTTS_WILLMSG       0x09,  // Connect Acknowledgment
  MQTTS_REGISTER      0x0A,  // Connect Acknowledgment
  MQTTS_REGACK        0x0B,  // Connect Acknowledgment
  MQTTS_PUBLISH       0x0C,  // Publish message
  MQTTS_PUBACK        0x0D,  // Publish Acknowledgment
  MQTTS_PUBCOMP       0x0E,  // Publish Received (assured delivery part 1)
  MQTTS_PUBREC        0x0F,  // Publish Release (assured delivery part 2)
  MQTTS_PUBREL        0x10,  // Publish Complete (assured delivery part 3)
  //RESERVED          0x11,  
  MQTTS_SUBSCRIBE     0x12,  // Client Subscribe request
  MQTTS_SUBACK        0x13,  // Subscribe Acknowledgment
  MQTTS_UNSUBSCRIBE   0x14,  // Client Unsubscribe request
  MQTTS_UNSUBACK      0x15,  // Unsubscribe Acknowledgment
  MQTTS_PINGREQ       0x16,  // PING Request
  MQTTS_PINGRESP      0x17,  // PING Response
  MQTTS_DISCONNECT    0x18,  // Client is Disconnecting
  //RESERVED          0x19,  
  MQTTS_WILLTOPICUPD  0x1A,  // Connect Acknowledgment
  MQTTS_WILLTOPICRESP 0x1B,  // Connect Acknowledgment
  MQTTS_WILLMSGUPD    0x1C,  // Connect Acknowledgment
  MQTTS_WILLMSGRESP   0x1D,  // Connect Acknowledgment
  //RESERVED          0x1E-0xFD, 
  MQTTS_ENCAPSULATED  0xFE,  // Encapsulated message
  //RESERVED          0xFF,  // reserved
}

#define MQTTS_QOS0        (0 << 1)
#define MQTTS_QOS1        (1 << 1)
#define MQTTS_QOS2        (2 << 1)

class mqttClient {
private:
   uint8_t buffer[MQTTS_MAX_PACKET_SIZE];
   uint16_t nextMsgId;
   unsigned long lastOutActivity;
   unsigned long lastInActivity;
   bool pingOutstanding;
   void (*callback)(char*,uint8_t*,unsigned int);
   uint16_t readPacket();
   uint8_t readByte();
   boolean write(uint8_t header, uint8_t* buf, uint16_t length);
   uint16_t writeString(char* string, uint8_t* buf, uint16_t pos);
   IPAddress ip;
   char* domain;
   uint16_t port;
   char* clientId;
   char* username;
   char* password;
   char* willTopic;
   uint8_t willQos;
   uint8_t willRetain;
   char* willMessage;
public:
   mqttClient();
   // TODO mqttClient(void(*)(char*,uint8_t*,unsigned int), Client& client);
   mqttClient(IPAddress& ip, uint16_t, void(*)(char*,uint8_t*,unsigned int), Client& client);
   mqttClient(String&, uint16_t, void(*)(char*,uint8_t*,unsigned int), Client& client);
   boolean connect(char *);
   boolean connect(char *, char *, char *);
   boolean connect(char *, char *, uint8_t, uint8_t, char *);
   boolean connect(char *, char *, char *, char *, uint8_t, uint8_t, char*);
   boolean connect();
   void disconnect();
   boolean publish(char *, char *);
   boolean publish(char *, uint8_t *, unsigned int);
   boolean publish(char *, uint8_t *, unsigned int, boolean);
   boolean publish_P(char *, uint8_t PROGMEM *, unsigned int, boolean);
   boolean subscribe(char *);
   boolean loop();
   boolean connected();
};


#endif /* _PINOCCIO_MQTTS_CLIENT_H */