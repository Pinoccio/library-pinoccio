#ifndef _PINOCCIO_MESH_REQUEST_H_
#define _PINOCCIO_MESH_REQUEST_H_

#include <Arduino.h>

class MeshRequest {
  public:    
    MeshRequest();
    void setDstAddress(uint16_t address);
    void setDstEndpoint(uint8_t endpoint);
    void setSrcEndpoint(uint8_t endpoint);
    void setOptions(uint8_t options);
    void setPayload(byte *data, uint8_t size);
    void setConfirmCallback(void (*confirm)(struct NWK_DataReq_t *req));
    
    NWK_DataReq_t* getRequest();
  protected:  
    NWK_DataReq_t request;    
};

#endif // _PINOCCIO_MESH_REQUEST_H_