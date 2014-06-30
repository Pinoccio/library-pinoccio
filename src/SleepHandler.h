#ifndef LIB_PINOCCIO_SLEEP_H
#define LIB_PINOCCIO_SLEEP_H

class SleepHandler {
  public:
    static void setup();

    // Sleep until the given time, in ticks. If interruptible is true,
    // this can return earlier if we are woken from sleep by another
    // interrupt.
    //
    // An until_ticks of 2^31 (±9.5 hour) or more ticks in the future will be
    // treated as if it was in the past and no sleeping happens.
    static void doSleep(uint32_t until_ticks, bool interruptible);

    // A timer tick is always 16μs, so the tick count overflows after
    // 2^32 * 16 / 1000 == 68,719,476 ms (±19 hours). ms should not be
    // bigger than that.
    static uint32_t msToTicks(uint32_t ms) {
      return ms * 1000 / US_PER_TICK;
    }
    static uint32_t usToTicks(uint32_t us) {
      return us / US_PER_TICK;
    }
    static uint32_t ticksToMs(uint32_t ticks) {
      return ticks / 1000 * US_PER_TICK + ticks % 1000 * US_PER_TICK / 1000;
    }

    // Maximum time that a single sleep can last (±9.5 hours)
    static const uint32_t MAX_SLEEP_TICKS = (1 << 31);

    // Returns the number of ticks since startup
    static uint32_t ticks() {
      return read_sccnt();
    }

  protected:
    static bool sleepUntilMatch(bool interruptible);
    static uint32_t read_sccnt();
    static void write_scocr3(uint32_t val);

    // The symbol counter always runs at 62.5kHz (period 16μs). When running
    // off the 16Mhz crystal, this is just a prescaler. When running of the
    // 32kiHz crystal, has some special circuitry to end up at 62.5Khz on
    // average (which probably leads to a bit of jitter, but the long-term
    // average frequency should not be affected).
    static const uint16_t CLOCK = 62500;
    static const uint8_t US_PER_TICK = (1000000/62500);
    // TODO: Once C++11 is enabled, add a static_assert to check that
    // US_PER_TICK is integer
};

#endif // LIB_PINOCCIO_SLEEP_H
