/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2012-2014, Pinoccio Inc. All rights reserved.              *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the BSD License as described in license.txt.         *
\**************************************************************************/
#include <Arduino.h>
#include <stddef.h>
#include <util/crc16.h>
#include "../../Shell.h"
#include "OtaModule.h"
#include "wibo.h"
#include <lwm/phy/phy.h>
#include <lwm/nwk/nwk.h>
#include <lwm/nwk/nwkTx.h>

using namespace pinoccio;

OtaModule OtaModule::instance;

// 802.15.4 MAC layer header
const uint16_t MAC802154_FRAME_TYPE_MASK = 0x7;
const uint16_t MAC802154_FRAME_TYPE_SHIFT = 0;
const uint16_t MAC802154_FRAME_TYPE_BEACON = 0;
const uint16_t MAC802154_FRAME_TYPE_DATA = 1;
const uint16_t MAC802154_FRAME_TYPE_ACK = 2;
const uint16_t MAC802154_FRAME_TYPE_COMMAND = 3;

const uint16_t MAC802154_SECURITY_ENABLED = 0x8;
const uint16_t MAC802154_FRAME_PENDING = 0x10;
const uint16_t MAC802154_ACK_REQUEST = 0x20;
const uint16_t MAC802154_INTRA_PAN = 0x40;
const uint16_t MAC802154_ADDR_MODE_MASK = 0x3;
const uint16_t MAC802154_DST_ADDR_MODE_SHIFT = 10;
const uint16_t MAC802154_SRC_ADDR_MODE_SHIFT = 14;
const uint16_t MAC802154_ADDR_MODE_NOT_PRESENT = 0;
const uint16_t MAC802154_ADDR_MODE_SHORT = 2;
const uint16_t MAC802154_ADDR_MODE_EXTENDED = 3;

// Frame Control Field value for all P2P frames
const uint16_t P2P_FCF =
   (MAC802154_FRAME_TYPE_DATA << MAC802154_FRAME_TYPE_SHIFT)
   | MAC802154_INTRA_PAN
   | (MAC802154_ADDR_MODE_SHORT << MAC802154_DST_ADDR_MODE_SHIFT)
   | (MAC802154_ADDR_MODE_SHORT << MAC802154_SRC_ADDR_MODE_SHIFT);


const uint16_t TIMEOUT = 250; // ms
// Room for data is 127 - header size - 2 bytes FCS
const uint8_t DATA_PACKET_SIZE = NWK_FRAME_MAX_PAYLOAD_SIZE - sizeof(p2p_wibo_data_t) - 2;

// When cloning, use this block size (e.g. do a CRC check every this
// many bytes).
const uint16_t CLONE_BLOCK_SIZE = 4096;
const uint32_t BOOTLOADER_SIZE = 8192; // 4096 words
const uint32_t CLONE_TOTAL_SIZE = FLASHEND + 1 - BOOTLOADER_SIZE;

// How often to try stuff
const uint8_t MAX_TRIES = 5;

// Wait this many ms before retrying, to be more resilient against
// bursts of radio interference
const uint8_t RETRY_DELAY = 50;

// This is the flash page size, the bootloader writes out data whenever
// it receives this many bytes.
const uint16_t BLOCK_ALIGN = 256;

// When the target is writing to flash, it stops processing packets. It
// still receives (and acks :-S) packets, but just doesn't process them.
// To prevent packets being left unprocessed, we delay after a packet
// that causes a page write. This delay is the maximum amount of time
// spent erasing (8.9ms) and programming (4.5ms), minus the typical
// transmission time of a full packet (127+6 bytes * 8 bits / 250kbps = 4ms).
// This means that the delay will be over before the target finishes
// programming the flash. However, the target should be finished by the
// time the second packet is completely received, which is good enough.
// It's a pity we have to harcode this, but the only alternative would
// be to send ping requests until the target replies, which probably
// results in an even bigger overhead. Perhaps a future bootloader
// version can handle this more gracefully.
const uint32_t DATA_FLASH_DELAY = 10;

enum class MemType {
  NOP = 'X',
  FLASH = 'F',
};

/** Memory type to use */
MemType memtype = MemType::FLASH;

enum class State : uint8_t {
  IDLE,
  PINGREQ, // ota.ping -> send ping
  START_PING, // ota.start -> send ping
  START_MEMORY, // ota.start -> select flash memory
  // ota.clone uses BLOCK_* as well, but with block_data = NULL
  BLOCK_ADDRESS, // ota.block -> send address
  BLOCK_DATA, // ota.block -> send data
  BLOCK_PING, // ota.block -> final ping
  BLOCK_DONE, // ota.block -> done (maybe failed), do cleanup
  END, // ota.end -> send reboot
};

enum class TxState : uint8_t {
  IDLE,
  SENDING,
  SENT,
  REPLIED,
  RETRY,
};

// Current state
State state = State::IDLE;
static TxState txstate = TxState::IDLE;

// Time of last TX packet
static uint32_t txtime;

/** Short address of target */
uint16_t target = 2;

// The block of data as received through serial
static uint8_t *block_data = NULL;
// How big is the block (must be a multiple of BLOCK_ALIGN)
static uint16_t block_size = 0;
// How much padding must be added to the block?
static uint16_t block_padding = 0;
// How many bytes were sent of this block?
static uint16_t block_sent = 0;
// Start address for this block in target's flash
static uint32_t block_addr;
// How often did we retry resending the entire block?
static uint8_t block_tries = 0;
// How often did we retry resending the package?
static uint8_t tx_tries = 0;

// The CRC the target should have if everything was processed ok
static uint16_t expected_crc = 0;
// The address that we think the target will write the next byte to
static uint32_t target_memory_addr = -1;

/****************************\
 * Radio TX/RX functions
\****************************/
static void handle_ping_reply(p2p_ping_cnf_t *p);

extern "C" bool PHY_PreDataInd(PHY_DataInd_t *ind) {
  // Not looking for a ping? Leave the packet for LWM to process.
  if (state != State::PINGREQ
      && state != State::START_PING
      && state != State::BLOCK_PING)
    return false;

  if (txstate != TxState::SENT)
    return false;

  // Long enough to be a ping reply?
  if (ind->size < sizeof(p2p_ping_cnf_t))
    return false;

  p2p_ping_cnf_t *p = (p2p_ping_cnf_t*)ind->data;

  // Bootloader always uses short addressing, without security within
  // the same PAN.
  if (p->hdr.fcf != P2P_FCF)
    return false;

  // Only accept from our target, or from anyone if we broadcasted
  if (target != 0xffff && p->hdr.src != target)
    return false;

  // Must be unicast to us
  if (p->hdr.dst == 0xffff)
    return false;

  // If we get here, we've probably received a message from the
  // bootloader. It can still be an accidental LWM packet if the target
  // isn't in bootloader mode properly yet, so still be careful.

  // Check that appname is nul-terminated (there might be extra nuls before
  // the last byte, the apname field is fixed-size).
  if (p->appname[sizeof(p->appname) - 1] != '\0')
    return false;

  // Check that the boardname is nul-terminated
  size_t namelen = ind->size - offsetof(p2p_ping_cnf_t, boardname);
  if (p->boardname[namelen - 1] != '\0')
    return false;

  handle_ping_reply(p);
  return true;
}

static void handle_tx_fail();

static void tx_confirm_callback(NwkFrame_t *frame) {
  if (frame->tx.status == NWK_SUCCESS_STATUS) {
    txstate = TxState::SENT;
    nwkFrameFree(frame);
    return;
  }

  if (frame->tx.status == NWK_PHY_NO_ACK_STATUS) {
    speol("No ack received");
  } else {
    speol("TX failed");
    speol(frame->tx.status);
  }
  handle_tx_fail();

  nwkFrameFree(frame);
}

static void tx_frame(uint16_t dst, NwkFrame_t *frame, size_t size) {
  // Set flags to precent nwkTxFrame from trying to do routing or
  // encryption...
  frame->tx.control = NWK_TX_CONTROL_DIRECT_LINK | NWK_TX_CONTROL_ROUTING;
  frame->tx.confirm = tx_confirm_callback;
  frame->size = size;

  // HACK: For direct link frames, nwkTxFrame copies the MAC dst address
  // from the NWK dst address a bit further on in the frame. Therefore,
  // we set our intended address there. Since there could be meaningful
  // data in that spot, we back that up first and restore it after
  // calling nwkTxFrame.
  // This works because nwkTxFrame doesn't actually send the frame, it
  // queues it for later transmission
  uint16_t tmp = frame->header.nwkDstAddr;
  frame->header.nwkDstAddr = dst;
  nwkTxFrame(frame);
  frame->header.nwkDstAddr = tmp;

  txstate = TxState::SENDING;
  txtime = millis();
}


static numvar send_ping(uint16_t dst) {
  NwkFrame_t *frame;
  frame = nwkFrameAlloc();
  if (!frame) {
    speol(F("Failed to send ping"));
    return 0;
  }

  p2p_ping_req_t *p = (p2p_ping_req_t*)&frame->data;
  p->hdr.cmd = P2P_PING_REQ;

  tx_frame(dst, frame, sizeof(*p));
  frame->header.macFcf &= ~MAC802154_ACK_REQUEST;

  return 1;
}

static numvar send_select_memory(uint16_t dst, MemType memtype) {
  NwkFrame_t *frame;
  frame = nwkFrameAlloc();
  if (!frame) {
    speol(F("Failed to select memory"));
    return 0;
  }

  p2p_wibo_target_t *p = (p2p_wibo_target_t*)&frame->data;
  p->hdr.cmd = P2P_WIBO_TARGET;
  p->targmem = (uint8_t)memtype;

  tx_frame(target, frame, sizeof(*p));

  return 1;
}

static numvar send_set_address(uint16_t dst, uint32_t address) {
  NwkFrame_t *frame;
  frame = nwkFrameAlloc();
  if (!frame) {
    speol(F("Failed to send set address"));
    return 0;
  }

  p2p_wibo_addr_t *p = (p2p_wibo_addr_t*)&frame->data;
  p->hdr.cmd = P2P_WIBO_ADDR;
  p->address = address;

  tx_frame(dst, frame, sizeof(*p));

  return 1;
}

static numvar send_data(uint16_t dst, uint8_t *buf, uint32_t flash_addr, uint8_t size) {
  NwkFrame_t *frame;
  frame = nwkFrameAlloc();
  if (!frame) {
    speol(F("Failed to send ping"));
    return 0;
  }

  p2p_wibo_data_t *p = (p2p_wibo_data_t*)&frame->data;
  p->hdr.cmd = P2P_WIBO_DATA;
  p->dsize = size;
  for (uint8_t i = 0; i < size; ++i) {
    uint8_t b;
    if (buf)
      b = buf[i];
    else
      b = pgm_read_byte_far(flash_addr + i);

    expected_crc = _crc_ccitt_update(expected_crc, b);
    p->data[i] = b;
  }

  tx_frame(dst, frame, sizeof(*p) + size);

  return 1;
}

static numvar send_exit(uint16_t dst) {
  NwkFrame_t *frame;
  frame = nwkFrameAlloc();
  if (!frame) {
    speol(F("Failed to send reset"));
    return 0;
  }

  p2p_wibo_exit_t *p = (p2p_wibo_exit_t*)&frame->data;
  p->hdr.cmd = P2P_WIBO_EXIT;

  tx_frame(dst, frame, sizeof(*p));

  return 1;
}

/****************************\
 * Bitlash commands
\****************************/

static numvar ping() {
  if (!checkArgs(1, F("usage: ota.ping(addr)"))) {
    return 0;
  }

  if (state != State::IDLE) {
    speol(F("Ota operation in progress?"));
    speol(F("FAIL"));
    return 0;
  }

  target = getarg(1);
  state = State::PINGREQ;
  tx_tries = 0;

  return 1;
}

static numvar start() {
  if (!checkArgs(2, F("usage: ota.start(addr, dryrun)"))) {
    return 0;
  }

  if (state != State::IDLE) {
    speol(F("Ota operation in progress?"));
    speol(F("FAIL"));
    return 0;
  }

  target = getarg(1);
  memtype = getarg(2) ? MemType::NOP : MemType::FLASH;

  if (!send_ping(target))
    return 0;

  // Force sending a set address packet on the first block
  target_memory_addr = -1;

  state = State::START_PING;
  tx_tries = 0;
}

static numvar block() {
  if (!checkArgs(2, F("usage: ota.data(memaddr, hexdata) or ota.data(memaddr, size) followed by \\0 and size binary data bytes"))) {
    return 0;
  }

  if (state != State::IDLE) {
    speol(F("Ota operation in progress?"));
    speol(F("FAIL"));
    return 0;
  }

  block_addr = getarg(1);
  if (isstringarg(2)) {
    size_t len = strlen((const char*)getstringarg(2));
    if (len % 2) {
      speol(F("Uneven hex digits"));
      return 0;
    }
    block_size = len / 2;
  } else {
    block_size = getarg(2);
  }


  // Since the bootloader only writes when a full flash page is
  // received, we must make sure that blocks are always a multiple of
  // this. If we would not do this, we can't be sure that data was
  // actualy be written to flash (the crc is updated for data received,
  // not written). Furthermore, a "set address" packet throws away any
  // received-but-not-yet-written data. Easiest way around this is to
  // just make blocks a multiple of this size. For optimal performance,
  // callers should make sure that all blocks sent are already properly
  // sized.
  uint16_t misalign = block_size % BLOCK_ALIGN;
  uint16_t padding = (BLOCK_ALIGN - misalign) % BLOCK_ALIGN;

  // Alloc memory
  block_data = (uint8_t*)malloc(block_size + padding);
  if (!block_data) {
    speol(F("Memory allocation failed"));
    speol(F("FAIL"));
    return 0;
  }

  uint32_t i = 0;
  if (isstringarg(2)) {
    const char *str = (const char*)getstringarg(2);
    for (;i < block_size; ++i) {
      if (!isHexadecimalDigit(str[i*2]) || !isHexadecimalDigit(str[i*2+1])) {
        speol(F("Invalid hex digit"));
        speol(F("FAIL"));
        free(block_data);
        block_data = NULL;
        return 0;
      }
      block_data[i] = PinoccioShell::parseHex(str[i*2]) << 4 | PinoccioShell::parseHex(str[i*2+1]);
    }
  } else {
    // There might be a \n in the buffer still
    if (Serial.peek() == '\n')
      Serial.read();

    // Expect a nul byte, to prevent ambiguity with the \n above
    if (Serial.peek() != 0) {
      speol(F("Data must start with NUL byte"));
      speol(F("FAIL"));
      free(block_data);
      block_data = NULL;
      return 0;
    }

    uint32_t start = millis();
    const uint32_t timeout = 10000;
    while (i < block_size) {
      if (Serial.available()) {
        block_data[i++] = Serial.read();
      } else if (millis() - start > timeout) {
        sp(F("Timeout reading data from serial ("));
        sp(i);
        speol(" bytes read)");
        speol(F("FAIL"));
        return 0;
      }
    }
  }

  // Add padding
  block_size += padding;
  for (; i < block_size; ++i)
    block_data[i] = 0xff;

  if (block_addr != target_memory_addr)
    state = State::BLOCK_ADDRESS;
  else
    state = State::BLOCK_DATA;
  block_tries = 0;
  block_sent = 0;
  tx_tries = 0;
}

static numvar clone() {
  if (!checkArgs(0, F("usage: ota.clone"))) {
    return 0;
  }

  if (state != State::IDLE) {
    speol(F("Ota operation in progress?"));
    speol(F("FAIL"));
    return 0;
  }

  state = State::BLOCK_ADDRESS;
  block_data = NULL;
  block_size = CLONE_BLOCK_SIZE;
  block_sent = 0;
  block_tries = 0;
  block_addr = 0;
  tx_tries = 0;
}

static numvar end() {
  if (!checkArgs(0, F("usage: ota.end"))) {
    return 0;
  }

  if (state != State::IDLE) {
    speol(F("Ota operation in progress?"));
    speol(F("FAIL"));
    return 0;
  }

  state = State::END;
  tx_tries = 0;
}

/****************************\
 *   MODULE CLASS STUFF     *
\****************************/

const __FlashStringHelper *OtaModule::name() const {
  return F("ota");
}

bool OtaModule::enable() {
  Shell.addFunction("ota.ping", ping);
  Shell.addFunction("ota.start", start);
  Shell.addFunction("ota.block", block);
  Shell.addFunction("ota.clone", clone);
  Shell.addFunction("ota.end", end);

  return true;
}

void OtaModule::loop() {
  switch (txstate) {
    case TxState::RETRY:
      if (millis() - txtime < RETRY_DELAY)
        break;
      /* falllthrough */
    case TxState::IDLE:
    {
      bool res = true;
      switch (state) {
        case State::IDLE:
          // Nothing to do
          break;
        case State::PINGREQ:
          res = send_ping(target);
          break;
        case State::START_PING:
          res = send_ping(target);
          break;
        case State::START_MEMORY:
          res = send_select_memory(target, memtype);
          break;
        case State::BLOCK_ADDRESS:
          res = send_set_address(target, block_addr);
          break;
        case State::BLOCK_DATA:
        {
          uint8_t size = min(block_size - block_sent, DATA_PACKET_SIZE);
          if (block_data) // ota.block from memory
            res = send_data(target, block_data + block_sent, 0, size);
          else // ota.clone from flash
            res = send_data(target, NULL, block_addr + block_sent, size);
          break;
        }
        case State::BLOCK_PING:
          res = send_ping(target);
          break;
        case State::BLOCK_DONE:
          // Block is done, do cleanup (status was already printed)
          free(block_data);
          block_data = NULL;
          state = State::IDLE;
          txstate = TxState::IDLE;
          break;
        case State::END:
          // Note that we don't send the finish message. It only flushes
          // unwritten data, which we should never have because we send
          // in multiples of the flash size. Also, there is no real way
          // to confirm that the finish was actually received... Instead
          // we assume all bytes were written to flash and make the
          // target reboot instead.
          res = send_exit(target);
          break;
      }

      if (!res) {
        speol();
        speol("FAIL");
        state = State::IDLE;
      }
      break;
    }
    case TxState::SENDING:
      /* Wait for ack / tx confirm */
      break;

    case TxState::SENT:
      switch (state) {
        case State::IDLE: // Should not happen
        case State::BLOCK_DONE: // Should not happen
          break;
        case State::PINGREQ:
        case State::START_PING:
        case State::BLOCK_PING:
          // Waiting for ping reply, treat timeout as TX fail
          if (millis() - txtime > TIMEOUT) {
            speol(F("No ping reply"));
            if (state == State::PINGREQ) {
              // Don't retry
              state = State::IDLE;
              txstate = TxState::IDLE;
            } else {
              // possibly retry
              handle_tx_fail();
            }
          }
          break;
        case State::START_MEMORY:
          /* ota.start done */
          state = State::IDLE;
          txstate = TxState::IDLE;
          speol();
          speol("OK");
          break;
        case State::BLOCK_ADDRESS:
          // Address sent, send first block of data
          state = State::BLOCK_DATA;
          txstate = TxState::IDLE;
          target_memory_addr = block_addr;
          break;
        case State::BLOCK_DATA:
        {
          uint8_t prev_pagenum = block_sent / BLOCK_ALIGN;

          // Packet was sent, update counters
          uint8_t size = min(block_size - block_sent, DATA_PACKET_SIZE);
          block_sent += size;
          target_memory_addr += size;

          // If the previous data packet completed a flash page, the
          // target will be busy writing it to flash. It does not
          // process packets during this time (but _does_ ACK them :-S),
          // so we hardcode a delay here...
          if (prev_pagenum != block_sent / BLOCK_ALIGN)
            delay(DATA_FLASH_DELAY);

          // Check if there's more data to send
          if (block_sent < block_size) {
            // Send next packet
            txstate = TxState::IDLE;
          } else {
            // Done, check crc
            state = State::BLOCK_PING;
            txstate = TxState::IDLE;
          }
          break;
        }
        case State::END:
          state = State::IDLE;
          txstate = TxState::IDLE;
          speol();
          speol("OK");
          break;
      }
      break;

    // Note: State change after ping reply is handled 

    default:
      break;
  }
}

static void handle_ping_reply(p2p_ping_cnf_t *p) {
  txstate = TxState::IDLE;
  switch (state) {
    case State::PINGREQ: {
      StringBuffer msg(100);
      msg.concat(F("PONG "));
      msg.appendSprintf("{'short_addr':0x%04X, 'status':0x%02X, 'errno':0x%04X,"
                        "'appname': '%s', 'boardname':'%s', 'version':0x%02X, "
                        "'crc':0x%04X}", p->hdr.src, p->status, p->errno,
                        p->appname, p->boardname, p->version, p->crc);

      speol();
      speol(msg);
      state = State::IDLE;
      break;
    }
    case State::START_PING:
      // Record current CRC for the first block
      expected_crc = p->crc;
      state = State::START_MEMORY;
      // Update our target in case we did a broadcast ping
      target = p->hdr.src;
      break;
    case State::BLOCK_PING:
      if (expected_crc == p->crc) {
        if (block_data) {
          // ota.block command, we're done
          state = State::BLOCK_DONE;
          speol();
          speol(F("OK"));
        } else {
          // ota.clone command, advance to the next block
          block_addr += block_size;
          block_sent = 0;
          block_size = min(CLONE_BLOCK_SIZE, CLONE_TOTAL_SIZE - block_addr);
          tx_tries = 0;
          block_tries = 0;
          if (block_size) {
            // Start next block
            state = State::BLOCK_DATA;
          } else {
            // Done!
            state = State::IDLE;
            speol();
            speol(F("OK"));
          }
        }
      } else {
        // Failed to send block, retry
        speol(F("Crc mismatch"));
        if (block_tries++ >= MAX_TRIES) {
          speol();
          speol(F("FAIL"));
          state = State::BLOCK_DONE;
        } else {
          speol(F("Retrying block"));
          state = State::BLOCK_ADDRESS;
          txstate = TxState::RETRY;
          txtime = millis();
          block_sent = 0;
          expected_crc = p->crc;
        }
      }
      break;
    case State::IDLE:
    case State::START_MEMORY:
    case State::BLOCK_ADDRESS:
    case State::BLOCK_DATA:
    case State::BLOCK_DONE:
    case State::END:
      /* N/A */
      break;
  }
}

static void handle_tx_fail() {
  switch (state) {
    case State::IDLE: // Should not occur
    case State::BLOCK_DONE: // Should not occur
      break;

    case State::PINGREQ:
    case State::START_PING:
    case State::START_MEMORY:
    case State::BLOCK_PING:
    case State::BLOCK_ADDRESS:
    case State::BLOCK_DATA:
    case State::END:
      // Retry
      if (tx_tries++ < MAX_TRIES) {
        txstate = TxState::RETRY;
        speol(F("Retrying TX"));
      } else {
        speol();
        speol(F("FAIL"));
        txstate = TxState::IDLE;
        switch (state) {
          case State::BLOCK_PING:
          case State::BLOCK_ADDRESS:
          case State::BLOCK_DATA:
            // Cleanup
            state = State::BLOCK_DONE;
            break;
          default:
            state = State::IDLE;
            break;
            break;
        }
      }
      break;
  }
}
