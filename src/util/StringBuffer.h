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

#ifndef LIB_PINOCCIO_UTIL_STRINGBUFFER_H_
#define LIB_PINOCCIO_UTIL_STRINGBUFFER_H_

#include <Arduino.h>
#include <Client.h>

/**
 * Extend the Arduino String class with some operations that make using
 * it as buffer for reading input or building output more efficient
 * (e.g,. without copying data).
 */
class StringBuffer : public String {
  public:
    // Only constructor takes an initial size and allocation block size
    // Block size should be a power-of-2.
    //
    // If you need to initialize a Stringbuffer with a value, use the
    // assignment operator, e.g.,
    //   StringBuffer buf(100, 16);
    //   buf = "foo";
    StringBuffer(size_t initial = 0, size_t block_size = 1);

    // Helper methods
    size_t appendSprintf(const char *fmt, ...);
    int readClient(Client& c, size_t size);
    bool blockReserve(size_t size);

    // Explicitly include String's operator=, since the implicitly
    // defined one for StringBuffer hides these by default.
    using String::operator=;

  protected:
    size_t block_size_mask;
};

#endif // LIB_PINOCCIO_UTIL_STRINGBUFFER_H_
