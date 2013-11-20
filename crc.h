#ifndef _CRC_H
#define _CRC_H

#include <stdint.h>

uint8_t pinoccio_crc_update(const uint8_t poly, uint8_t crc, uint8_t data);
#endif // CRC_H
