#ifndef LIB_PINOCCIO_CRC_H
#define LIB_PINOCCIO_CRC_H

#include <stdint.h>

uint8_t pinoccio_crc_update(const uint8_t poly, uint8_t crc, uint8_t data);
#endif // LIB_PINOCCIO_CRC_H
