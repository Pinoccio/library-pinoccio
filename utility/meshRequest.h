#ifndef _PINOCCIO_MESH_REQUEST_H_
#define _PINOCCIO_MESH_REQUEST_H_

#include <Arduino.h>

#ifdef NWK_ENABLE_SECURITY
  #define APP_BUFFER_SIZE     NWK_MAX_SECURED_PAYLOAD_SIZE
#else
  #define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

enum
{
  NWK_PAYLOAD_HEADER_RESPONSE  = 1 << 0,
  NWK_PAYLOAD_HEADER_FINAL     = 1 << 1,
};

typedef struct NWK_PayloadHeader_t {
  uint8_t control;
} NWK_PayloadHeader_t;

class MeshRequest {
  public:
    MeshRequest();
    void setDstAddress(uint16_t address);
    void setDstEndpoint(uint8_t endpoint);
    void setSrcEndpoint(uint8_t endpoint);
    void setOptions(uint8_t options);
    void setHeader(uint8_t headerData);

    int setPayload(byte *data, uint8_t size);
    void setConfirmCallback(void (*confirm)(struct NWK_DataReq_t *req));

    NWK_DataReq_t* getRequest();

  protected:
    uint8_t header;
    NWK_DataReq_t request;
};

#endif // _PINOCCIO_MESH_REQUEST_H_