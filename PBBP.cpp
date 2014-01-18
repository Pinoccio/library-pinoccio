#include "PBBP.h"
#include "crc.h"
#include <Arduino.h>

PBBP::PBBP() {
  this->num_slaves = 0;
  this->slave_ids = NULL;
  this->last_error = OK;
}

void PBBP::begin(uint8_t pin) {
  this->pin = pin;
  pinMode(this->pin, INPUT);
}

bool PBBP::enumerate() {
  uint8_t b;
  this->num_slaves = 0;
  if (!sendReset() || !sendByte(BC_CMD_ENUMERATE)) {
    if (this->last_error == NO_ACK_OR_NACK) {
      // Nobody on the bus
      return true;
    }
    // Other error
    return false;
  }

  while (this->num_slaves < this->max_slaves) {
    // Allocate room to store one more address
    this->slave_ids = (uint8_t (*)[UNIQUE_ID_LENGTH]) realloc(this->slave_ids, (this->num_slaves + 1) * sizeof(this->slave_ids[0]));
    uint8_t *id = this->slave_ids[this->num_slaves];
    uint8_t crc = 0;
    for (uint8_t i = 0; i < UNIQUE_ID_LENGTH; ++i) {
      if (!receiveByte(&id[i])) {
        if (i == 0 && this->last_error == NO_ACK_OR_NACK) {
          // Nobody responded, meaning all device are enumerated
          return true;
        }
        // Other error
        return false;
      }
      crc = pinoccio_crc_update(UNIQUE_ID_CRC_POLY, crc, id[i]);
    }

    if (crc != 0) {
      this->last_error = CRC_ERROR;
      return false;
    }

    this->num_slaves++;
  }

  // See if there is one more
  if (receiveByte(&b)) {
    // Succesfully received a byte, there are more slaves!
    this->last_error = TOO_MANY_SLAVES;
    return false;
  } else {
    return (this->last_error == NO_ACK_OR_NACK);
  }
}

bool PBBP::sendReset() {
  if (!this->waitForFreeBus())
    return false;
  this->bit_start = micros();
  pinMode(this->pin, OUTPUT);
  digitalWrite(this->pin, LOW);
  while (micros() - this->bit_start < this->reset_time) /* wait */;
  pinMode(this->pin, INPUT);
  if (!this->waitForFreeBus())
    return false;
  delayMicroseconds(this->idle_time);
  // Make sure the next bit doesn't wait, nor complain that too much
  // time has passed since the previous bit
  this->bit_start = 0;
  return true;
}

bool PBBP::sendByte(uint8_t b) {
  bool parity_val = 0;
  bool ok = true;
  uint8_t next_bit = 0x80;
  while (next_bit && ok) {
    if (b & next_bit)
      parity_val ^= 1;
    if (!sendBit(b & next_bit))
      return false;
    next_bit >>= 1;
  }

  return sendBit(!parity_val) && receiveReady() && receiveAck();
}

bool PBBP::sendBytes(const uint8_t *buf, uint8_t len) {
  while (len--) {
    if (!sendByte(*buf++))
      return false;
  }
  return true;
}

bool PBBP::receiveByte(uint8_t *b) {
  bool parity_val = 0;
  bool ok = true;
  *b = 0;
  uint8_t next_bit = 0x80;
  bool value;
  // Receive data bits
  while (next_bit && ok) {
    if (!receiveBit(&value))
      return false;

    if (value) {
      *b |= next_bit;
      parity_val ^= 1;
    }
    next_bit >>= 1;
  }
  // Receive parity bit
  if (!receiveBit(&value))
    return false;

  if (value == parity_val) {
    this->last_error = PARITY_ERROR;
    return false;
  }

  return receiveReady() && receiveAck();
}

bool PBBP::receiveBytes(uint8_t *buf, uint8_t len) {
  while (len--) {
    if (!receiveByte(buf++))
      return false;
  }
  return true;
}

bool PBBP::sendCommand(uint8_t slave_addr, uint8_t command) {
  return sendReset() &&
         sendByte(slave_addr) &&
         sendByte(command);
}

bool PBBP::readEeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t *buf, uint8_t len) {
  return sendCommand(slave_addr, CMD_READ_EEPROM) &&
         sendByte(eeprom_addr) &&
         receiveBytes(buf, len);
}

bool PBBP::writeEeprom(uint8_t slave_addr, uint8_t eeprom_addr, const uint8_t *buf, uint8_t len) {
  return sendCommand(slave_addr, CMD_WRITE_EEPROM) &&
         sendByte(eeprom_addr) &&
         sendBytes(buf, len);
}

bool PBBP::waitForFreeBus() {
  // TODO: A proper timeout
  uint8_t timeout = 255;
  while(timeout--) {
    if (digitalRead(this->pin) == HIGH)
      return true;
  }

  this->last_error = TIMEOUT;
  return false;
}

bool PBBP::waitForNextBitStart() {
  if (bit_start != 0) {
    // Make sure that at least idle_time passed since the end of the
    // previous bit
    while(micros() - bit_end < this->idle_time) /* wait */;

    // Make sure that at least next_bit_time passed since the start of the
    // previous bit
    while(micros() - bit_start < this->next_bit_time) /* wait */;

    // If this bit starts too late, the slave will have timed out already
    if (micros() - bit_start > this->max_next_bit_time) {
      this->last_error = BIT_TOO_LATE;
      return false;
    }
  }
  this->bit_start = micros();
  return true;
}

bool PBBP::sendBit(bool value) {
  if (!waitForNextBitStart() || !waitForFreeBus())
    return false;
  pinMode(this->pin, OUTPUT);
  while (micros() - this->bit_start < this->start_time) /* wait */;
  if (value)
    pinMode(this->pin, INPUT);
  while (micros() - this->bit_start < this->value_time) /* wait */;
  pinMode(this->pin, INPUT);
  this->bit_end = micros();
  return true;
}

bool PBBP::receiveBit(bool *value) {
  if (!waitForNextBitStart() || !waitForFreeBus())
    return false;
  pinMode(this->pin, OUTPUT);
  while (micros() - this->bit_start < this->start_time) /* wait */;
  pinMode(this->pin, INPUT);
  while (micros() - this->bit_start < this->sample_time) /* wait */;
  *value = digitalRead(this->pin);
  while (micros() - this->bit_start < this->value_time) /* wait */;
  // If a slave pulls the line low, wait for him to finish (to
  // prevent the idle time from disappearing because of a slow
  // slave), but don't wait forever.
  if (!waitForFreeBus())
    return false;
  this->bit_end = micros();
  return true;
}

bool PBBP::receiveReady() {
  int timeout = this->max_stall_bits;
  while (timeout--) {
    bool value;
    if (!receiveBit(&value))
      return false;

    // Ready bit?
    if (value)
      return true;
  }
  this->last_error = STALL_TIMEOUT;
  return false;
}

bool PBBP::receiveAck() {
  bool first, second;
  if (!receiveBit(&first) || !receiveBit(&second))
    return false;

  // Acks are sent as 01, nacks as 10. Since the 0 is dominant during
  // a bus conflict, a receiveing of 00 means both an ack and a nack was
  // sent.
  if (!first && !second ) {
    this->last_error = ACK_AND_NACK;
    return false;
  } else if (!second ) {
    // Read error code from the slave
    if (receiveByte(&this->last_slave_error)) {
      this->last_error = NACK;
    } else {
      this->last_error = NACK_NO_SLAVE_CODE;
    }
    return false;
  } else if (first) {
    this->last_error = NO_ACK_OR_NACK;
    return false;
  } else {
    return true;
  }
}

const char *PBBP::error_code_str[] = {
  [OK] = "OK",
  [STALL_TIMEOUT] = "STALL_TIMEOUT",
  [TIMEOUT] = "TIMEOUT",
  [NACK] = "NACK",
  [NACK_NO_SLAVE_CODE] = "NACK_NO_SLAVE_CODE",
  [NO_ACK_OR_NACK] = "NO_ACK_OR_NACK",
  [ACK_AND_NACK] = "ACK_AND_NACK",
  [PARITY_ERROR] = "PARITY_ERROR",
  [BIT_TOO_LATE] = "BIT_TOO_LATE",
  [CRC_ERROR] = "CRC_ERROR",
  [TOO_MANY_SLAVES] = "TOO_MANY_SLAVES",
};

void PBBP::printLastError(Print &p) {
    if (this->last_error < sizeof(error_code_str) / sizeof(*error_code_str))
        p.print(error_code_str[this->last_error]);
    else
        p.print(this->last_error);

    if (this->last_error == NACK) {
        p.print(", slave error code: 0x");
        p.print(this->last_slave_error, HEX);
    }
}

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
