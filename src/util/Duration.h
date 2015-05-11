#ifndef LIB_PINOCCIO_DURATION_H
#define LIB_PINOCCIO_DURATION_H

class Duration {
  public:
    // Number of whole seconds in this duration
    uint32_t seconds;

    // Number of microseconds within the current second. This always
    // has a value of 0 <= us < 1,000,000.
    //
    // 20 bits would have room up to 1,048,576, but adding a few extra
    // bits allows headroom for at least one overflow during
    // calculations.
    uint32_t us:24;

    Duration operator +(uint64_t us_increment) {
      Duration result(*this);
      result.seconds += us_increment / 1000000;
      result.us += us_increment % 1000000;
      if (result.us >= 1000000) {
        result.us -= 1000000;
        result.seconds++;
      }
      return result;
    }

    Duration& operator +=(uint64_t us_increment) {
      *this = *this + us_increment;
    }

    Duration operator -(Duration d) {
      Duration result(*this);
      result.seconds -= d.seconds;
      if (result.us < d.us) {
        result.us += 1000000;
        result.seconds--;
      }
      result.us -= d.us;

      return result;
    }

    Duration operator +(Duration d) {
      Duration result(*this);
      result.seconds += d.seconds;
      result.us += d.us;
      if (result.us > 1000000) {
        result.us -= 1000000;
        result.seconds++;
      }

      return result;
    }
};

#endif // LIB_PINOCCIO_DURATION_H
