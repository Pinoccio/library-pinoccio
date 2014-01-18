#ifndef LIB_PINOCCIO_CRC_H
#define LIB_PINOCCIO_CRC_H

#include <stdint.h>

uint8_t pinoccio_crc_update(const uint8_t poly, uint8_t crc, uint8_t data);
uint8_t pinoccio_crc_generate_byte(const uint8_t poly, uint8_t crc, uint8_t *data, uint16_t length);
uint16_t pinoccio_crc_generate_word(const uint16_t poly, uint16_t crc, uint8_t *data, uint16_t length);
#endif // LIB_PINOCCIO_CRC_H
