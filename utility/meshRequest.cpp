#include <Arduino.h>
#include <Pinoccio.h>
#include "meshRequest.h"

MeshRequest::MeshRequest() {
  request.dstEndpoint = 1;
  request.srcEndpoint = 1;
  request.options = NWK_OPT_ENABLE_SECURITY;
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

void MeshRequest::setPayload(byte *data, uint8_t size) {
  request.data = (uint8_t*)data;
  request.size = size;
}

void MeshRequest::setConfirmCallback(void (*confirm)(struct NWK_DataReq_t *req)) {
  request.confirm = confirm;
}

NWK_DataReq_t* MeshRequest::getRequest() {
  return &request;
}