// Class for handling minifloat values
#ifndef LIB_PINOCCIO_MINIFLOAT_H
#define LIB_PINOCCIO_MINIFLOAT_H

#include <math.h>
#include "integer.h"
#include "Arduino.h"
#include "static_assert.h"

// This is jus a bunch of macros that the compiler automatically defines
// that should be said like this when the "float" type actually uses
// IEEE754 32-bit floats, as the below code assumes. If this error
// triggers, this might mean something is wrong, or perhaps your
// compiler just doesn't define all of these...
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ || __FLT_HAS_DENORM__ != 1 || \
    __FLT_MANT_DIG__ != 24 || __FLT_RADIX__ != 2 || __FLT_MIN_EXP__ != (-125) || \
    __FLT_MAX_EXP__ != 128
#error "Cannot confirm that IEEE754 floats are actually used"
#endif

/**
 * Custom floating point format, intendeded for small floating point
 * numbers. Allows automatically converting to the native "float" type
 * for further processing.
 *
 * TODO: internal memory structure uses a native-endian (little-endian
 * on AVR) layout, so for multi-byte floats, this struct can't just be
 * overlayed on a big-endian formatted minifloat.
 */
template <unsigned exponent_bits, unsigned significand_bits, int exponent_bias>
struct Minifloat {
  // raw_t is the smallest integer type that can fit all bits of this minifloat.
  typedef typename boost::uint_t<significand_bits + exponent_bits>::least raw_t;
  // sig_t is the smallest integer type that can fit the significand bits
  typedef typename boost::uint_t<significand_bits>::least sig_t;
  // exp_t is the smallest integer type that can fit the raw exponent bits
  typedef typename boost::uint_t<exponent_bits>::least raw_exp_t;
  // exp_t is the smallest integer type that can fit the actual (biased)
  // exponent bits. This assumes that the biased exponent will never be
  // more than twice as big as the unbiased exponent. TODO: Ideally,
  // we'd use boost::int_max_value_t and boost::int_min_value_t, but
  // those rely on limits.h which avr does not supply.
  typedef typename boost::int_t<exponent_bits + 2>::least exp_t;

  Minifloat(raw_t val) {
    this->val = val;
  }

  operator float() const {
    static const int extra_significand_bits = IEEE754_SIGNIFICAND_BITS - significand_bits;
    static const int exponent_bias_offset = IEEE754_EXPONENT_BIAS - exponent_bias;
    static const int from_max_exp = (1 << exponent_bits) - 1 - exponent_bias;
    static const int to_max_exp = (1 << IEEE754_EXPONENT_BITS) - 1 - IEEE754_EXPONENT_BIAS;
    static const int from_min_exp = 1 - exponent_bias;
    static const int to_min_exp = 1 - IEEE754_EXPONENT_BIAS;
    // We want to do lossless conversion, so we first need to check that
    // the range of our minifloat is actually less than float (e.g., can
    // we fit the minifloat inside a regular float?).
    // The range of the signficand is the same for both: [1, 2> (for
    // normal numbers), just with differing precision. We don't want to
    // lose any precision, so the number significand bits should be
    // greater or equal.
    static_assert(extra_significand_bits >= 0, "To many significand bits for lossless conversion to float");
    // Now, we look at the range of the exponent (after biasing). The
    // maximum exponent of our minifloat format should not be more than
    // that of float, since then the biggest minifloat numbers are not
    // representable as a float. Note that this can even happen when
    // exponent_bits is small, if the bias is much smaller that float's.
    static_assert(from_max_exp <= to_max_exp, "Biggest exponent too high for lossless conversion to float");

    // On the bottom range of the exponent, things get more complicated.
    // If float has more significand bits, then those extra bits can be
    // used to increase the exponent range a bit by shifting the
    // exponent to the right (but not that this is just a small amount:
    // every extra significand bit gives 1 extra exponent, while every
    // exponent bit _doubles_ the number of exponents).
    static_assert(from_min_exp >= to_min_exp - extra_significand_bits, "Smallest exponent too low for lossless conversion to float");

    ieee754_float res;
    res.sign = 0;

    // Now, the actual conversion. How to approach this depends on the
    // lowest exponents of both formats.
    //
    // First, the easiest case is where both formats have the same
    // minimal exponent:
    //   source exponent range:        |--------|
    //   target exponent range:        |----------------|
    // In this case, we can just copy the significand (padding with
    // zeroes at the right) and the exponent (padding with zeroes at the
    // left). Normal numbers can remain normal numbers and denormal
    // numbers remain denormal numbers.
    if (from_min_exp == to_min_exp) {
      res.significand = (uint32_t)raw_significand() << extra_significand_bits;
      res.exponent = raw_exponent() + exponent_bias_offset;
    }

    // The next case is when the target minimal exponent is smaller than
    // the source one:
    //   source exponent range:            |--------|
    //   target exponent range:        |----------------|
    if (from_min_exp > to_min_exp) {
      if (raw_exponent() > 0) {
        // Normal numbers can still be copied as-is,
        res.significand = (uint32_t)raw_significand() << extra_significand_bits;
        res.exponent = raw_exponent() + exponent_bias_offset;
      } else {
        // Denormal numbers
        if (raw_significand() == 0) {
          // Special case: zero
          res.significand = 0;
          res.exponent = 0;
        } else {
          sig_t significand = raw_significand();
          // Non-zero denormal numbers will have the wrong exponent. To
          // fix this, we shift the significand to the left until
          // either:
          //  - We shift out a leading 1, making it a normal number
          //  - We reach the target minimal exponent
          res.exponent = 1 + exponent_bias_offset;
          bool done = false;
          do {
            if (exponent_bias_offset < significand_bits) {
              // Small optimization: We can be certain that a "1" is
              // shifted out after significand_bits shifts. If the
              // exponent bias offset is equal to that or more, then we
              // can be sure that the exponent will never reach 1 and we
              // can skip this check.
              if (res.exponent == 1) {
                // We reached the minimum exponent value before a 1 was
                // shifted out on the left. This means the number can
                // remain a denormal number. Set the exponent to 0 to
                // indicate this.
                res.exponent = 0;
                break;
              }
            }

            // Shift left until a "1" is about to be shifted out. If so,
            // stop _after_ shifting off that one.
            if ((significand & (1L << (significand_bits - 1))))
              done = true;

            significand <<= 1;
            res.exponent--;
          } while (!done);
          res.significand = (uint32_t)significand << extra_significand_bits;
        }
      }
    }

    // The last case is when the source minimal exponent is smaller than
    // the target one:
    //   source exponent range:    |--------|
    //   target exponent range:        |----------------|
    if (from_min_exp < to_min_exp) {
      if (raw_exponent() > 0) {
        // Normal number
        if (raw_exponent() >= to_min_exp + exponent_bias) {
          // Normal numbers whose exponent is more or equal to the target
          // minimal exponent can still be represented as-is. Note that
          // we do "to_min_exp + exponent_bias" instead of
          // "raw_exponent() - exponent_bias", since the latter would
          // overflow.
          res.significand = (uint32_t)raw_significand() << extra_significand_bits;
          res.exponent = raw_exponent() + exponent_bias_offset;
        } else {
          // Normal numbers with a smaller exponent need to turned into
          // denormal numbers and be shifted right until their exponent
          // becomes the target minimal exponent (which is what is used
          // for denormal numbers).
          uint8_t shift_right = (to_min_exp - (raw_exponent() - exponent_bias));
          res.significand = (uint32_t)raw_significand() << (extra_significand_bits - shift_right);
          // Also add the (previously implicit) leading 1 bit
          res.significand |= 1L << (IEEE754_SIGNIFICAND_BITS - shift_right);
          res.exponent = 0;
        }
      } else {
        // Denormal numbers remain denormal, but are also shifted right
        // to make their exponent match the target minimal exponent.
        const int shift_right = to_min_exp - from_min_exp;
        //static_assert(to_min_exp >= from_min_exp || to_min_exp - from_min_exp
        res.significand = (uint32_t)raw_significand() << (extra_significand_bits - shift_right);
        res.exponent = 0;
      }
    }

    return res.f;
  }

  sig_t raw_significand() const {
    return val & ((1 << significand_bits) - 1);
  }

  exp_t exponent() const {
    raw_exp_t e = raw_exponent();
    return (e == 0 ? (exp_t)1 : (exp_t)raw_exponent) - exponent_bias;
  }

  raw_exp_t raw_exponent() const {
    return (val >> significand_bits) & ((1 << exponent_bits) - 1);
  }

  raw_t raw() const {
    return val;
  }

protected:
  raw_t val;

  static const int IEEE754_EXPONENT_BIAS = 127;
  static const int IEEE754_SIGNIFICAND_BITS = 23;
  static const int IEEE754_EXPONENT_BITS = 8;
  union ieee754_float {
    float f;
    struct {
      uint32_t significand: IEEE754_SIGNIFICAND_BITS;
      uint32_t exponent: IEEE754_EXPONENT_BITS;
      uint32_t sign: 1;
    };
  };
};

/* Some code used to test the above implementation

  // Test by storing 5.75, 0.875 (denormal) and 0.0 and printing out
  // the result.
  Serial.println(ldexp(Minifloat<4, 4, 0>(0x27), 0), 3);
  Serial.println(ldexp(Minifloat<4, 4, 0>(0x07), 0), 3);
  Serial.println(ldexp(Minifloat<4, 4, 0>(0x0), 0), 3);

  // Next, test with different biases to activate different parts of the
  // conversion code. Undo the biasing with ldexp for printing to get
  // the original numbers again.
  Serial.println(ldexp(Minifloat<4, 4, 127>(0x27), 127), 3);
  Serial.println(ldexp(Minifloat<4, 4, 127>(0x07), 127), 3);
  Serial.println(ldexp(Minifloat<4, 4, 127>(0x0), 127), 3);

  Serial.println(ldexp(Minifloat<4, 4, 126>(0x27), 126), 3);
  Serial.println(ldexp(Minifloat<4, 4, 126>(0x07), 126), 3);
  Serial.println(ldexp(Minifloat<4, 4, 126>(0x0), 126), 3);

  Serial.println(ldexp(Minifloat<4, 4, 20>(0x27), 20), 3);
  Serial.println(ldexp(Minifloat<4, 4, 20>(0x07), 20), 3);
  Serial.println(ldexp(Minifloat<4, 4, 20>(0x0), 20), 3);

  Serial.println(ldexp(Minifloat<4, 4, 129>(0x27), 129), 3);
  Serial.println(ldexp(Minifloat<4, 4, 129>(0x07), 129), 3);
  Serial.println(ldexp(Minifloat<4, 4, 129>(0x0), 129), 3);

  Serial.println(ldexp(Minifloat<4, 4, 128>(0x27), 128), 3);
  Serial.println(ldexp(Minifloat<4, 4, 128>(0x07), 128), 3);
  Serial.println(ldexp(Minifloat<4, 4, 128>(0x0), 128), 3);
*/

#endif // LIB_PINOCCIO_MINIFLOAT_H

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
