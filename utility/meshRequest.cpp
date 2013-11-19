#include <Arduino.h>
#include <Pinoccio.h>
#include "meshRequest.h"

MeshRequest::MeshRequest() {
  request.dstEndpoint = 1;
  request.srcEndpoint = 1;
  request.options = 0;
  //TODO: request.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;

  header = 0x00;
}

void MeshRequest::setDstAddress(uint16_t address) {
  request.dstAddr = address;
}

void MeshRequest::setDstEndpoint(uint8_t endpoint) {
  request.dstEndpoint = endpoint;
}

void MeshRequest::setSrcEndpoint(uint8_t endpoint) {
  request.srcEndpoint = endpoint;
}

void MeshRequest::setOptions(uint8_t options) {
  request.options = options;
}

void MeshRequest::setHeader(uint8_t headerData) {
  header = headerData;
}

int MeshRequest::setPayload(byte *data, uint8_t size) {
  if (size > APP_BUFFER_SIZE - 1) {
    return -1;
  }
  byte payload[APP_BUFFER_SIZE];

  payload[0] = header;
  memcpy((void *)payload[1], (const void *)data, size);
  request.data = (uint8_t*)payload;
  request.size = size + 1;

  return 0;
}

void MeshRequest::setConfirmCallback(void (*confirm)(struct NWK_DataReq_t *req)) {
  request.confirm = confirm;
}

NWK_DataReq_t* MeshRequest::getRequest() {
  return &request;
}