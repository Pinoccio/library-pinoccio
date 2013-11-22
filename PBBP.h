// Class for communicating using the Pinoccio Backpack Bus protocol
#ifndef _PINOCCIO_PBBP_H
#define _PINOCCIO_PBBP_H

#include <stdint.h>
#include <Print.h>
#include "PBBP-protocol.h"

class PBBP {
public:
  PBBP();
  void begin(uint8_t pin);
  bool enumerate();
  bool sendReset();
  bool sendByte(uint8_t b);
  bool receiveByte(uint8_t *b);

  bool readEeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t *buf, uint8_t len);
  bool writeEeprom(uint8_t slave_addr, uint8_t eeprom_addr, const uint8_t *buf, uint8_t len);

  void printLastError(Print &p);

  enum error {
    OK,
    STALL_TIMEOUT,
    TIMEOUT,
    NACK,
    // NACK received, but failed to receive error code
    NACK_NO_SLAVE_CODE,
    NO_ACK_OR_NACK,
    ACK_AND_NACK,
    PARITY_ERROR,
    BIT_TOO_LATE,
    // CRC error in a received unique id
    CRC_ERROR,
    // More slaves than available addresses
    TOO_MANY_SLAVES,
  };

  // The slaves detected during enumeration
  uint8_t num_slaves;
  uint8_t (*slave_ids)[UNIQUE_ID_LENGTH];

  // Last error that occured. Only valid when the last method called
  // returned false.
  error last_error;

  // If last_error is NACK, this is the error code sent by the
  // slave
  uint8_t last_slave_error;

protected:
  bool waitForFreeBus();
  bool waitForNextBitStart();
  bool sendBit(bool value);
  bool receiveBit(bool *value);
  bool receiveReady();
  bool receiveAck();
  
  uint8_t pin;

  // The start time (in micros) of the current or previous bit
  unsigned long bit_start;
  // The end time (in micros) of the previous bit
  unsigned long bit_end;

  static const uint16_t reset_time = 2500;
  static const uint16_t start_time = 125;
  static const uint16_t value_time = 650;
  static const uint16_t sample_time = 350;
  static const uint16_t idle_time = 50;
  static const uint16_t next_bit_time = 700;
  static const uint16_t max_next_bit_time = 1100;

  static const uint8_t max_stall_bits = 20;
  
  static const uint8_t max_slaves = 127;

  static const char *error_code_str[];
};

#endif // _PINOCCIO_PBBP_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
