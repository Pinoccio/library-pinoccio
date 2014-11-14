/* Copyright (c) 2007 - 2012 Axel Wachtler
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIB_PINOCCIO_WIBO_H_
#define LIB_PINOCCIO_WIBO_H_

// Contents copied from the uracoli project

// OTA ("P2P") commands, from uracoli
// generic
#define P2P_PING_REQ (0x01)          /**< Ping a node which replies and delivers
                                          information about itself */
#define P2P_PING_CNF (0x02)          /**< Reply to a ping request */
#define P2P_JUMP_BOOTL (0x03)        /**< forces the application to jump to
                                          bootloader */
// wibo
#define P2P_WIBO_DATA (0x20)          /**< Feed a node with data */
#define P2P_WIBO_FINISH (0x21)        /**< Force a write of all received data */
#define P2P_WIBO_RESET (0x22)         /**< Reset data stream */
#define P2P_WIBO_EXIT (0x23)          /**< Exit bootloader and jump to
                                           application vector */
#define P2P_WIBO_TARGET (0x24)        /**< Set programming target */

#define P2P_WIBO_TARGET (0x24)        /**< Set programming target */
#define P2P_WIBO_DEAF (0x25)          /**< Put node to deaf (no reply to ping) */
#define P2P_WIBO_ADDR (0x26)          /**< Set address */
#define P2P_WIBO_BOOTLUP (0x27)       /**< Initiate bootloader update */

/* === Types ================================================================ */

enum class p2p_status_t : uint8_t {
    P2P_STATUS_IDLE = 0x00,
    P2P_STATUS_RECEIVINGDATA = 0x01,
    P2P_STATUS_ERROR = 0xFF
};

enum class p2p_error_t : uint8_t {
    P2P_ERROR_NONE = 0x00,
    P2P_ERROR_NONE_DATAMISS,
    P2P_ERROR_SUCCESS
};

/**
 * Frame header structure for P2P applications.
 * This header is formated as a IEEE 802.15.4 frame header with short addressing
 * mode for src and dst and pan comression set to on.
 */
typedef struct
{
    uint16_t fcf;  /**< frame control field */
    uint8_t  seq;  /**< sequence number */
    uint16_t pan;  /**< destination pan id */
    uint16_t dst;  /**< destination short address */
    uint16_t src;  /**< source short address */
    uint8_t cmd;   /**< the command code that identifies the frame,
                        see @ref p2p_cmdcode_t */
} p2p_hdr_t;


/** Frame structure for @ref P2P_PING_REQ. */
typedef struct
{
    p2p_hdr_t hdr;
} p2p_ping_req_t;


/** Frame structure for @ref P2P_PING_CNF. */
typedef struct
{
    p2p_hdr_t hdr;
    p2p_status_t status;  /**< application status */
    p2p_error_t errno;    /**< current error code */
    uint8_t version;      /**< software version */
    uint16_t crc;         /**< checksum of received data (only used from WIBO)*/
    char appname[16];      /**< application identification string */
    char boardname[];     /**< board identification string */
} p2p_ping_cnf_t;

/** Frame structure for @ref P2P_JUMP_BOOTL. */
typedef struct
{
    p2p_hdr_t hdr;
} p2p_jump_bootl_t;

/** Frame structure for @ref P2P_WIBO_DATA. */
typedef struct {
    p2p_hdr_t hdr;
    uint8_t dsize;   /**< size of data packet */
    uint8_t data[];  /**< data container */
} p2p_wibo_data_t;

/** Frame structure for @ref P2P_WIBO_FINISH. */
typedef struct
{
    p2p_hdr_t hdr;
} p2p_wibo_finish_t;


/** Frame structure for @ref P2P_WIBO_RESET. */
typedef struct
{
    p2p_hdr_t hdr;
} p2p_wibo_reset_t;

/** Frame structure for @ref P2P_WIBO_TARGET. */
typedef struct
{
    p2p_hdr_t hdr;
    uint8_t targmem; /**< target memory: EEPROM or FLASH or LOCKBITS */
} p2p_wibo_target_t;

/** Frame structure for @ref P2P_WIBO_ADDR. */
typedef struct
{
    p2p_hdr_t hdr;
    uint32_t address;
} p2p_wibo_addr_t;

/** Frame structure for @ref P2P_WIBO_EXIT. */
typedef struct
{
    p2p_hdr_t hdr;
} p2p_wibo_exit_t;

#endif
