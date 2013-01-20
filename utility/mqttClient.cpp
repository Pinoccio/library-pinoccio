/*
mqttClient.cpp - A simple client for MQTT.
Nicholas O'Leary
http://knolleary.net
*/

#include "mqttClient.h"
#include <string.h>
#include "IPAddress.h"

mqttClient::mqttClient() { }

mqttClient::mqttClient(IPAddress& ip, uint16_t port, void (*callback)(char*,uint8_t*,unsigned int), Client& client) {
  this->_client = &client;
  this->callback = callback;
  this->ip = ip;
  this->port = port;
}

mqttClient::mqttClient(String& domain, uint16_t port, void (*callback)(char*,uint8_t*,unsigned int), Client& client) {
  this->_client = &client;
  this->callback = callback;
  domain.toCharArray(this->domain, domain.length());
  this->port = port;
}

boolean mqttClient::connect(char *id) {
  return connect(id,NULL,NULL,0,0,0,0);
}

boolean mqttClient::connect(char *id, char *user, char *pass) {
  return connect(id,user,pass,0,0,0,0);
}

boolean mqttClient::connect(char *id, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage)
{
  return connect(id,NULL,NULL,willTopic,willQos,willRetain,willMessage);
}

boolean mqttClient::connect(char *id, char *user, char *pass, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage) {
  Serial.println("DEBUG: mqttClient::connect 1");
  
  if (!connected()) {
    int result = 0;
    
    Serial.println("DEBUG: mqttClient::connect 2");
    if (domain != NULL) {
      result = _client->connect(this->domain, this->port);
    } else {
      result = _client->connect(this->ip, this->port);
    }

    Serial.println("DEBUG: mqttClient::connect 3");
    if (result) {
      Serial.println("DEBUG: mqttClient::connect 4");
      nextMsgId = 1;
      uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p',MQTTPROTOCOLVERSION};
      // Leave room in the buffer for header and variable length field
      uint16_t length = 5;
      unsigned int j;
      for (j = 0;j<9;j++) {
        buffer[length++] = d[j];
      }

      uint8_t v;
      if (willTopic) {
        v = 0x06|(willQos<<3)|(willRetain<<5);
      } else {
        v = 0x02;
      }

      if(user != NULL) {
        v = v|0x80;

        if(pass != NULL) {
          v = v|(0x80>>1);
        }
      }

      buffer[length++] = v;

      buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
      buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
      length = writeString(id,buffer,length);
      if (willTopic) {
        length = writeString(willTopic,buffer,length);
        length = writeString(willMessage,buffer,length);
      }

      if(user != NULL) {
        Serial.println("DEBUG: mqttClient::connect 4.1");
        length = writeString(user,buffer,length);
        if(pass != NULL) {
          Serial.println("DEBUG: mqttClient::connect 4.2");
          length = writeString(pass,buffer,length);
        }
      }

      Serial.println("DEBUG: mqttClient::connect 5");
      write(MQTTCONNECT,buffer,length-5);
      Serial.println("DEBUG: mqttClient::connect 6");
      
      lastInActivity = lastOutActivity = millis();

      while (!_client->available()) {
        unsigned long t = millis();
        if (t-lastInActivity > MQTT_KEEPALIVE*1000UL) {
          _client->stop();
          return false;
        }
      }
      Serial.println("DEBUG: mqttClient::connect 7");
      uint16_t len = readPacket();
      Serial.println("DEBUG: mqttClient::connect 8");
      if (len == 4 && buffer[3] == 0) {
        Serial.println("DEBUG: mqttClient::connect 9");
        lastInActivity = millis();
        pingOutstanding = false;
        return true;
      }
      Serial.println("DEBUG: mqttClient::connect 10");
    }
    _client->stop();
  }
  return false;
}

uint8_t mqttClient::readByte() {
  while(!_client->available()) {}
  return _client->read();
}

uint16_t mqttClient::readPacket() {
  uint16_t len = 0;
  buffer[len++] = readByte();
  uint8_t multiplier = 1;
  uint16_t length = 0;
  uint8_t digit = 0;
  do {
    digit = readByte();
    buffer[len++] = digit;
    length += (digit & 127) * multiplier;
    multiplier *= 128;
  } while ((digit & 128) != 0);

  for (uint16_t i = 0;i<length;i++)
  {
    if (len < MQTT_MAX_PACKET_SIZE) {
      buffer[len++] = readByte();
    } else {
      readByte();
      len = 0; // This will cause the packet to be ignored.
    }
  }

  return len;
}

boolean mqttClient::loop() {
  if (connected()) {
    unsigned long t = millis();
    if ((t - lastInActivity > MQTT_KEEPALIVE*1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE*1000UL)) {
      if (pingOutstanding) {
        _client->stop();
        return false;
      } else {
        buffer[0] = MQTTPINGREQ;
        buffer[1] = 0;
        _client->write(buffer,2);
        lastOutActivity = t;
        lastInActivity = t;
        pingOutstanding = true;
      }
    }
    if (_client->available()) {
      uint16_t len = readPacket();
      if (len > 0) {
        lastInActivity = t;
        uint8_t type = buffer[0]&0xF0;
        if (type == MQTTPUBLISH) {
          if (callback) {
            uint16_t tl = (buffer[2]<<8)+buffer[3];
            char topic[tl+1];
            for (uint16_t i=0;i<tl;i++) {
              topic[i] = buffer[4+i];
            }
            topic[tl] = 0;
                  // ignore msgID - only support QoS 0 subs
            uint8_t *payload = buffer+4+tl;
            callback(topic,payload,len-4-tl);
          }
        } else if (type == MQTTPINGREQ) {
          buffer[0] = MQTTPINGRESP;
          buffer[1] = 0;
          _client->write(buffer,2);
        } else if (type == MQTTPINGRESP) {
          pingOutstanding = false;
        }
      }
    }
    return true;
  }
  return false;
}

boolean mqttClient::publish(char* topic, char* payload) {
  return publish(topic,(uint8_t*)payload,strlen(payload),false);
}

boolean mqttClient::publish(char* topic, uint8_t* payload, unsigned int plength) {
  return publish(topic, payload, plength, false);
}

boolean mqttClient::publish(char* topic, uint8_t* payload, unsigned int plength, boolean retained) {
  if (connected()) {
      // Leave room in the buffer for header and variable length field
    uint16_t length = 5;
    length = writeString(topic,buffer,length);
    uint16_t i;
    for (i=0;i<plength;i++) {
      buffer[length++] = payload[i];
    }
    uint8_t header = MQTTPUBLISH;
    if (retained) {
      header |= 1;
    }
    return write(header,buffer,length-5);
  }
  return false;
}

boolean mqttClient::publish_P(char* topic, uint8_t* PROGMEM payload, unsigned int plength, boolean retained) {
  uint8_t llen = 0;
  uint8_t digit;
  int rc;
  uint16_t tlen;
  int pos = 0;
  int i;
  uint8_t header;
  unsigned int len;

  if (!connected()) {
    return false;
  }

  tlen = strlen(topic);

  header = MQTTPUBLISH;
  if (retained) {
    header |= 1;
  }
  buffer[pos++] = header;
  len = plength + 2 + tlen;
  do {
    digit = len % 128;
    len = len / 128;
    if (len > 0) {
      digit |= 0x80;
    }
    buffer[pos++] = digit;
    llen++;
  } while(len>0);

  pos = writeString(topic,buffer,pos);

  rc += _client->write(buffer,pos);

  for (i=0;i<plength;i++) {
    rc += _client->write((char)pgm_read_byte_near(payload + i));
  }

  lastOutActivity = millis();
  return rc == len + 1 + plength;
}

boolean mqttClient::write(uint8_t header, uint8_t* buf, uint16_t length) {
  uint8_t lenBuf[4];
  uint8_t llen = 0;
  uint8_t digit;
  uint8_t pos = 0;
  uint8_t rc;
  uint8_t len = length;
  do {
    digit = len % 128;
    len = len / 128;
    if (len > 0) {
      digit |= 0x80;
    }
    lenBuf[pos++] = digit;
    llen++;
  } while(len>0);

  buf[4-llen] = header;
  for (int i=0;i<llen;i++) {
    buf[5-llen+i] = lenBuf[i];
  }
  
  for (int j=0;j<length+1+llen;j++) {
    Serial.print((buf+(4-llen))[j], HEX);
  }
  Serial.println("");
  Serial.print("length: ");
  Serial.println(length+1+llen);
  
  rc = _client->write(buf+(4-llen),length+1+llen);

  lastOutActivity = millis();
  if (rc == 1+llen+length)
    Serial.print("Wrote all ");
  else {
    Serial.print("Only wrote ");
  }
  Serial.print(rc);
  Serial.println(" bytes");
  return (rc == 1+llen+length);
}


boolean mqttClient::subscribe(char* topic) {
  if (connected()) {
      // Leave room in the buffer for header and variable length field
    uint16_t length = 7;
    nextMsgId++;
    if (nextMsgId == 0) {
      nextMsgId = 1;
    }
    buffer[0] = (nextMsgId >> 8);
    buffer[1] = (nextMsgId & 0xFF);
    length = writeString(topic, buffer,length);
    buffer[length++] = 0; // Only do QoS 0 subs
    return write(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-5);
  }
  return false;
}

void mqttClient::disconnect() {
  buffer[0] = MQTTDISCONNECT;
  buffer[1] = 0;
  _client->write(buffer,2);
  _client->stop();
  lastInActivity = lastOutActivity = millis();
}

uint16_t mqttClient::writeString(char* string, uint8_t* buf, uint16_t pos) {
  char* idp = string;
  uint16_t i = 0;
  pos += 2;
  while (*idp) {
    buf[pos++] = *idp++;
    i++;
  }
  buf[pos-i-2] = (i >> 8);
  buf[pos-i-1] = (i & 0xFF);
  return pos;
}


boolean mqttClient::connected() {
  int rc = (int)_client->connected();
  if (!rc) _client->stop();
  return rc;
}
