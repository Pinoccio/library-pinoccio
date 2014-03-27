/*
 * Pinoccio Arduino Library - StringBuffer class
 *
 * Copyright (c) 2014 Matthijs Kooijman <matthijs@stdin.nl>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of Pinoccio nor the names of its contributors may be used
 *   to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Arduino.h>
#include "StringBuffer.h"

StringBuffer::StringBuffer(size_t initial, size_t block_size) {
  this->block_size_mask = block_size - 1;

  if (initial)
    blockReserve(initial);
}

bool StringBuffer::blockReserve(size_t size) {
  if (this->capacity > size)
    return true; // short circuit for the common case

  // Round up to a full block size
  size = (size + this->block_size_mask) & ~this->block_size_mask;
  return reserve(size);
}

size_t StringBuffer::appendSprintf(const char *fmt, ...)
{
  // Keep two copies of the args list, in case we need to call vsnprintf
  // twice
  va_list args, args2;
  va_start(args, fmt);
  va_copy(args2, args);

  size_t avail, len;
  if (this->buffer) {

    // Calculate the unused buffer size. This is the size for actual
    // characters, there's additionally room for the nul byte.
    avail = this->capacity - this->len;

    // Try to sprintf into the available buffer space (+ 1 since avail
    // excludes the room for the nul byte).
    len = vsnprintf(this->buffer + this->len, avail + 1, fmt, args);
  } else {
    // Special case: No buffer allocated yet. Call vsnprintf with a zero
    // size argument, so it won't write anything but just tells use how
    // much to allocate.
    avail = 0;
    len = vsnprintf(NULL, 0, fmt, args);
  }

  // Len returned is the string length that is, or would be, written,
  // excluding the nul byte. If that's more than avail, the buffer was
  // too small and we need to expand.
  if (len > avail) {
    blockReserve(this->len + len);
    if (!this->buffer)
      return 0;
    avail = this->capacity - this->len;
    len = vsnprintf(this->buffer + this->len, avail + 1, fmt, args2);
  }

  va_end(args2);
  va_end(args);

  // This shouldn't happen, but check anyway
  if (len > avail)
    len = avail;

  this->len += len;
  return len;
}

int StringBuffer::readClient(Client& c, size_t size) {
  // Make sure we have enough room
  blockReserve(this->len + size);
  int read = c.read((uint8_t *)this->buffer + this->len, size);
  this->len += read;
  this->buffer[this->len] = 0;
  return read;
}

size_t StringBuffer::appendJsonString(const char *in, size_t len, bool add_quotes) {
  const int ESCAPE_LEN = 6; // \uXXXX escape sequence is 6 bytes

  // First, allocate memory enough for to add the string as-is. This
  // should be enough if no characters need escaping
  size_t needed = this->len + len;

  if (add_quotes)
    needed += 2;

  if (!blockReserve(needed))
    return 0;

  size_t written = 0;

  if (add_quotes) {
    this->buffer[this->len] = '"';
    ++written;
  }

  // 2 tries should normally always work, but keep a count to guarantee
  // we can't get into an infinite loop
  uint8_t tries = 2;
  while(tries--) {
    char *out = this->buffer + this->len + written;
    size_t room = this->capacity - this->len - written;

    // Reserve space for the closing quote
    if (add_quotes)
      room--;

    // Then, copy over characters, replacing the JSON special ones with
    // \uXXXX escape sequences. Using \\, \", \n etc. where possible would
    // result in smaller JSON, but would require more code space, so we
    // just use the \uXXXX sequence for everything
    while(room && len) {
      char c = *in;
      if (c <= 0x1f || c == '\\' || c == '"') {
        // Escape
        if (room < ESCAPE_LEN)
          break;
        snprintf(out, ESCAPE_LEN + 1, "\\u%04x", c);
        room -= ESCAPE_LEN;
        out += ESCAPE_LEN;
        written += ESCAPE_LEN;
      } else {
        // Normal
        *out = c;
        ++out;
        --room;
        ++written;
      }
      ++in;
      --len;
    }

    if (len != 0 && tries) {
      // If there are still bytes left to write, we ran out of room.
      // Find out how many bytes we'll need.

      // Any bytes left in the input take up at least one byte
      needed += len;
      // Any room we have left (can happen when an escape sequence
      // didn't fit), we don't have to allocate again
      needed -= room;
      // Find out how many escape sequences left in the string
      size_t len2 = len;
      const char *in2 = in;
      while (len2--) {
        char c = *in2++;
        if (c <= 0x1f || c == '\\' || c == '"')
          // One byte was already accounted for above
          needed += ESCAPE_LEN - 1;
      }

      if (!blockReserve(needed))
        return 0;
    }
  }

  if (add_quotes) {
    this->buffer[this->len + written] = '"';
    ++written;
  }

  this->len += written;
  this->buffer[this->len] = 0;
  return written;
}

unsigned char StringBuffer::concat(const char *cstr, unsigned int length)
{
        unsigned int newlen = len + length;
        if (!cstr) return 0;
        if (length == 0) return 1;
        if (!reserve(newlen)) return 0;
        memcpy(buffer + len, cstr, length);
        len = newlen;
        return 1;
}

// vim: set sw=2 sts=2 expandtab:
