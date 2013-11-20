#ifndef _PINOCCIO_PBBP_PROTOCOL_H
#define _PINOCCIO_PBBP_PROTOCOL_H
// Broadcast commands (i.e., special addresses sent over the wire)
enum {
    // Start bus enumeration
    BC_CMD_ENUMERATE = 0xfe,
    BC_FIRST = BC_CMD_ENUMERATE,

    ADDRESS_RESERVED = 0xff,
};

// Targeted commands (i.e,. sent over the wire after the address)
enum {
    CMD_RESERVED = 0x00,
    CMD_READ_EEPROM = 0x01,
    CMD_WRITE_EEPROM = 0x02,

    CMD_FIRST = CMD_READ_EEPROM,
    CMD_LAST = CMD_WRITE_EEPROM,
};

uint8_t const UNIQUE_ID_LENGTH = 8;
uint8_t const UNIQUE_ID_CRC_POLY = 0x2f;

enum {
    ERR_OK = 0,
    ERR_OTHER = 1,
    ERR_PROTOCOL = 2,
    ERR_PARITY = 3,
    ERR_UNKNOWN_COMMAND = 4,

    ERR_WRITE_EEPROM_INVALID_ADDRESS = 0xff,
    ERR_WRITE_EEPROM_READ_ONLY = 0xfe,
    ERR_WRITE_EEPROM_FAILED = 0xfd,
    ERR_READ_EEPROM_INVALID_ADDRESS = 0xff,
};

#endif // _PINOCCIO_PBBP_PROTOCOL_H

/* vim: set filetype=cpp sw=4 sts=4 expandtab: */
