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
  Serial.println(size);
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
  return read;
}

// vim: set sw=2 sts=2 expandtab:
