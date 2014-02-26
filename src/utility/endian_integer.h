/*
 * Pinoccio Arduino Library - Fixed endian helper types
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
#ifndef LIB_PINOCCIO_ENDIANINT_H
#define LIB_PINOCCIO_ENDIANINT_H

/**
 * Helper class that allows storing an integer in memory using big
 * endian notation.
 *
 * @param T       The integer type to automatically cast to and from
 *                (must be at least bytes size long)
 * @param bytes   The size of this type in bytes. Does not need to be a
 *                power of two.
 */
template <typename T, int bytes = sizeof(T)>
class big_endian_integer {
public:
  // Don't define any constructors here (a constructor casting from T
  // would make sense), since that disallows using this type in an
  // (anonymous) union. Instead, we define an assignment operator which
  // also seems to do the job. C++11 will remove this restriction
  // http://stackoverflow.com/questions/5548636/what-are-unrestricted-unions-proposed-in-c11
  big_endian_integer& operator=(const T &v) {
    for (int i = 0; i < bytes; i++)
      val[i] = (v >> (bytes - i - 1) * 8) & 0xff;
  }

  // Transparently cast to T
  operator T() const {
    /*
    T res = 0;
    for (int i = 0; i < bytes; i++)
      res |= (T)val[i] << (bytes - i - 1) * 8;
    return res;
    */
    // TODO: The above code is better, because it works regardless of the
    // endianness of the current architecture (below code assumes little
    // endian). However, gcc fails to fully optimize away all overhead and
    // keeps some useless mov and or instructions, so we instead use below
    // code for now. http://gcc.gnu.org/bugzilla/show_bug.cgi?id=60145
    union {
      uint8_t arr[bytes];
      T v;
    } tmp = {0};
    for (int i = 0; i < bytes; i++)
      tmp.arr[bytes - i - 1] = val[i];
    return tmp.v;
  }

private:
  uint8_t val[bytes];
} __attribute__((packed));

typedef big_endian_integer<int64_t, 8> big64_t;
typedef big_endian_integer<uint64_t, 8> ubig64_t;
typedef big_endian_integer<int64_t, 7> big56_t;
typedef big_endian_integer<uint64_t, 7> ubig56_t;
typedef big_endian_integer<int64_t, 6> big48_t;
typedef big_endian_integer<uint64_t, 6> ubig48_t;
typedef big_endian_integer<int64_t, 5> big40_t;
typedef big_endian_integer<uint64_t, 5> ubig40_t;
typedef big_endian_integer<int32_t, 4> big32_t;
typedef big_endian_integer<uint32_t, 4> ubig32_t;
typedef big_endian_integer<int32_t, 3> big24_t;
typedef big_endian_integer<uint32_t, 3> ubig24_t;
typedef big_endian_integer<int16_t, 2> big16_t;
typedef big_endian_integer<uint16_t, 2> ubig16_t;
// For completness, but 8-bit types of course have no endianness.
typedef int8_t big8_t;
typedef uint8_t ubig8_t;

#endif // LIB_PINOCCIO_ENDIANINT_H
