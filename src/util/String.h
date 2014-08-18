/*
 * Pinoccio Arduino Library - String helper classes
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

#ifndef LIB_PINOCCIO_UTIL_STRING_H_
#define LIB_PINOCCIO_UTIL_STRING_H_

#include <Arduino.h>


// This file provides two classes, ConstString and ConstBuf. These
// classes can be used to wrap a const char* in an object.

/**
 * Class that wraps an existing char* string a String-like object so it
 * can be passed to functions expecting a const String&.
 *
 * ConstString is intended for a nul-terminated buffer and can be used
 * as if it is a regular (const) String object as well. To use it:
 *  - Let the called function accept a const String&:
 *      void func(const String&);
 *  - Pass in the char* converted to a ConstString object:
 *      func(ConstString("foo"));
 *
 * Note that you should not use the StringBuf instance after
 * reallocating, freeing of the original char* (modification is ok, if
 * the length doesn't change). Unfortunately, this cannot be detected at
 * compiletime or runtime, so be careful with non-temporary ConstString
 * objects.
 *
 *
 * Strings normally realloc the buffer if needed, but we can't allow
 * that. For this reason, ConstString can only ever be converted to a
 * const String&, never to a normal String&, which should guarantee the
 * buffer is never messed with.
 *
 * Note that you should ideally use this class only as a String&. If you
 * pass it to a function accepting a non-reference String, the String
 * copy constructor will trigger, making a complete copy of the entire
 * buffer.
 *
 * The ConstString class itself is not a (subclass of) String, since it
 * appears impossible to force to be used only as a const String& then.
 * This means that you cannot call string methods directly on a
 * ConstString, but you'll have to call them on the `s` member. e.g.,:
 *
 *   ConstString f("Foo");
 *   return f.s.length();
 *
 * The ConstString can be implicitely converted to a const String&, so
 * when passing to function, no extra steps are needed:
 */
class ConstString {
  // Internal class to store the actual buffer as a String object.
  class CString : public String {
    private:
      CString(const char *s, size_t len) {
        // Call parent constructor so it won't allocate a buffer
        String(NULL);
        // Remove the constness here, but since we are only exposed as a
        // constant object, the buffer should never be modified.
        this->buffer = const_cast<char*>(s);
        this->len = len;
      }

      ~CString() {
        // Prevent ~String from freeing the string
        this->buffer = NULL;
      }
    friend class ConstString;
    friend class ConstBuf;
  };
public:
  // Internal string object that can be used to access the String
  // methods for this string.
  const CString s;

  // Initialize from a "regular" (nul-terminated) char* string.
  // TODO: gcc doesn't seem to be inlining this constructor, which
  // prevents strlen from being evaluated at compiletime.
  ConstString (const char *str) : s(str, strlen(str)) { }

  // Intialize from a string when you already know its length. The
  // string must still be nul-terminated!
  ConstString (const char *str, size_t len) : s(str, len) { }

  // Allow implicitely converting to a const String&
  operator const String&() const {
    return this->s;
  }
};

/**
 * Class that wraps an existing buffer and its length in a conventient
 * object.
 *
 * ConstBuf is intended for buffers that are not nul-terminated and can
 * therefore _not_ be used as a regular String object. ConstBuf is
 * effectively an easy way to pass the buffer and its length in a
 * single variable.
 *
 * Regular (nul-terminated) strings and String objects can also be
 * implicitely converted to a ConstBuf object. To use ConstBuf:
 *
 *  - Let the called function accept a const ConstBuf&:
 *      void func(const ConstBuf&);
 *
 *  - Pass in a ConstBuf from a char* and length:
 *      char buf[] = {'a', 'b', 'c'};
 *      func(ConstBuf(buf, sizeof(buf)));
 *
 *  - Or pass in a char* or String directly:
 *      func("foo");
 *      String bar = "bar";
 *      func(bar);
 *
 * Note that you should not use the ConstBuf instance after reallocating
 * or freeing the original char* (modification is ok, if the length
 * doesn't change). When passing in a String, _any_ modification on the
 * String object invalidates the ConstBuf object. Unfortunately, this
 * cannot be detected at compiletime or runtime, so be careful with
 * non-temporary ConstBuf objects.
 */
class ConstBuf {
private:
  const uint8_t *buffer;
  size_t len;
public:
  // Allow access to the buffer and the length
  const uint8_t *buf() const { return this->buffer; }
  size_t length() const { return this->len; }
  // Allow implicitely casting to a char* and uint8_t*, so the buffer
  // can be accessed by using the object itself directly.
  operator const uint8_t *() const { return this->buffer; }

  // Initialize from a (non-terminated) buffer of the given length
  ConstBuf(const char *buf, size_t len) : buffer((const uint8_t*)buf), len(len) { }
  ConstBuf(const uint8_t *buf, size_t len) : buffer(buf), len(len) { }
  // Intialize from a regular (nul-terminated!) string
  ConstBuf(const char *str) : buffer((const uint8_t*)str), len(strlen(str)) { }
  // Initialized from a String object
  ConstBuf(const String& str) : buffer((const uint8_t*)str.c_str()), len(str.length()) { }
};

#endif // LIB_PINOCCIO_UTIL_STRING_H_
