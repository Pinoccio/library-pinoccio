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

#ifndef LIB_PINOCCIO_UTIL_CONCATENATOR_H_
#define LIB_PINOCCIO_UTIL_CONCATENATOR_H_

// This file provides a Concatenator class that can concatenate an
// arbitrary number of function arguments onto a String object,
// separated by an arbitrary separator value


// Helper class to append values to a String without any preprocessing
template <typename Value>
struct ValueToString {
  static void append(String &buf, Value value) {
    buf.concat(value);
  }
};

// Helper class to append values to a String, surrounded by quotes
template <typename Value>
struct ValueToQuotedString {
  static void append(String &buf, Value value) {
    String escaped(value);
    escaped.replace("\"","\\\"");
    buf.concat('"');
    buf.concat(escaped);
    buf.concat('"');
  }
};

// Helper class to append values to a String, surrounding strings (in
// various forms) with quotes and adding other values as-is.
template <typename Value> struct QuoteStringsOnly : ValueToString<Value> {};
template <> struct QuoteStringsOnly<const char*> : ValueToQuotedString<const char*> {};
template <> struct QuoteStringsOnly<char*> : ValueToQuotedString<char*> {};
template <> struct QuoteStringsOnly<String> : ValueToQuotedString<String> {};
template <> struct QuoteStringsOnly<char> : ValueToQuotedString<char> {};

template <typename Value> struct QuoteStringsAndFloats : QuoteStringsOnly<Value> {};
template <> struct QuoteStringsAndFloats<float> : ValueToQuotedString<float> {};

// Class to concatenate stuff. Use it by calling the static concat
// method on it. For example,
//
//   String result;
//   Concatenator<>::concat(result, ',', "foo", "bar", 123);
//
// Results in the string: foo,bar,123
//
// You can use other types for separators as well, and use different
// value preprocessing:
//
//   Concatenator<QuoteStringsOnly>::concat(result, ", ", "foo", "bar", 123);
//
// Results in: "foo", "bar", 123
//
// Note that only values are preprocessed, the separator is used as-is.
template < template<typename Value> class AppendValue = ValueToString >
struct  Concatenator {
  template <typename Sep>
  static void concat(String& buf, Sep sep) {
  }

  template <typename Sep, typename Arg>
  static void concat(String& buf, Sep sep, Arg arg) {
    AppendValue<Arg>::append(buf, arg);
  }

  template <typename Sep, typename Arg, typename... Args>
  static void concat(String& buf, Sep sep, Arg arg, Args... args...) {
    AppendValue<Arg>::append(buf, arg);
    buf.concat(sep);
    concat(buf, sep, args...);
  }
};

#endif // LIB_PINOCCIO_UTIL_CONCATENATOR_H_
