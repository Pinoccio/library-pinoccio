#ifndef LIB_PINOCCIO_SLEEP_H
#define LIB_PINOCCIO_SLEEP_H

#include "backpack-bus/Pbbe.h"
#include "util/Duration.h"

ISR(SCNT_OVFL_vect);

class SleepHandler {
  public:

    static void setOffset(uint64_t d);
    static uint64_t getOffset();

    static void setup();

    // Schedule a sleep until the given number of ms from now. The sleep
    // actually starts when doSleep is called, but any delay between
    // scheduleSleep and doSleep does not affect the end time of the
    // sleep (so it does affect the sleep duration).
    //
    // ms can not be 0, or more than 2^32 * 16 / 1000 ms (±19 hours), due to
    // the limited range of the counter used.
    static void scheduleSleep(uint32_t ms);

    // How many ticks are left before the end time is reached?
    static uint32_t scheduledTicksLeft();

    // If the previously scheduled end time has already passed, this
    // returns immediately, without sleeping.
    static uint32_t doSleep();

    static void setPinWakeup(uint8_t pin, bool enable);
    static bool pinWakeupSupported(uint8_t pin);
    static bool pinWakeupEnabled(uint8_t pin) {
      return Pbbe::LogicalPin(pin).in(pinWakeups);
    }

    // A timer tick is always 16μs, so the tick count overflows after
    // 2^32 * 16 / 1000 == 68,719,476 ms (±19 hours). ms should not be
    // bigger than that.
    static uint32_t msToTicks(uint32_t ms) {
      return ms / US_PER_TICK * 1000 + ms % US_PER_TICK * 1000 / US_PER_TICK;
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

    // Returns the total time since startup
    static Duration uptime() {
      // Atomically read the lastOverflow value
      SCIRQM &= ~(1 << IRQMOF);
      Duration last = lastOverflow;
      SCIRQM |= (1 << IRQMOF);
      return last + (uint64_t)read_sccnt() * US_PER_TICK;
    }

    // Returns the time slept since startup
    static const Duration& sleeptime() {
      return totalSleep;
    }

    // Returns the time awake since startup
    static Duration waketime() {
      return uptime() - totalSleep;
    }

    static uint32_t meshmicros();

  protected:

    // The mesh offset
    static uint64_t meshOffset;

    // The time from startup to the most recent overflow
    static Duration lastOverflow;
    // The total time spent sleeping since startup
    static Duration totalSleep;

    // Enable wakeups for changes on these pins
    static Pbbe::LogicalPin::mask_t pinWakeups;

    // Allow the symbol counter overflow ISR to access our protected members
    friend void SCNT_OVFL_vect();

    static void sleepUntilMatch();
    static uint32_t read_sccnt();
    static void write_scocr3(uint32_t val);
    static uint32_t read_scocr3();

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
