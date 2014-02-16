/*
 * Static assert without C++11
 *
 * Copyright (C) 2014 Matthijs Kooijman <matthijs@stdin.nl>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// This file defines some utility macros

#ifndef LIB_PINOCCIO_STATIC_ASSERT_H
#define LIB_PINOCCIO_STATIC_ASSERT_H

#if __cplusplus < 201103L && !defined(static_assert)
// C++11 defines this nice static_assert declaration, but otherwise
// emulate it (with less pretty error messages, but at least the checks
// are being done).
template<bool T> struct STATIC_ASSERTION;
template<> struct STATIC_ASSERTION<true> {typedef int SUCCESS; };

#define compiletime_concat(a, b) compiletime_concat2(a, b)
#define compiletime_concat2(a, b) a ## b
#define static_assert(condition, message) \
  typedef typename STATIC_ASSERTION<(bool)(condition)>::SUCCESS compiletime_concat(static_assert_, __LINE__ ) __attribute__((__unused__))
#endif // __cplusplus < 201103L && !defined(static_assert)

#endif // LIB_PINOCCIO_STATIC_ASSERT_H

// vim: set sw=2 sts=2 expandtab:
