#include <Arduino.h>
#include <avr/sleep.h>
#include "SleepHandler.h"

uint32_t SleepHandler::sleepMillis;

static bool timer_match = true;
ISR(TIMER2_COMPB_vect) {
  timer_match = true;
}

const uint16_t CLOCK = 32768;
const uint16_t PRESCALER = 1024;
const uint16_t MAX = 256;
const uint16_t TIMER_MAX_MS = 1000UL * MAX * PRESCALER / CLOCK;

static inline uint8_t MS_TO_TICKS(uint32_t ms) {
  return ms * (CLOCK / PRESCALER) / 1000;
}

static inline uint16_t TICKS_TO_MS(uint8_t ticks) {
  return (uint32_t)ticks * 1000 / (CLOCK / PRESCALER);
}

const uint8_t ASSR_BUSY_MASK = (1 << TCR2AUB) | (1 << TCR2BUB) | (1 << OCR2AUB) | (1 << OCR2BUB) | (1 << TCN2UB);

// Sleep until the timer match interrupt fired. If interruptible is
// true, this can return before if some other interrupt wakes us up
// from sleep. If this happens, true is returned.
bool SleepHandler::sleepUntilMatch(bool interruptible) {
  // When the timer is in asynchronous mode, it takes up to two
  // 32kHz clock cycles for register writes to take effect. Wait
  // until that's done before sleeping.
  while (ASSR & ASSR_BUSY_MASK) /* wait */;

  while (true) {
    #ifdef sleep_bod_disable
    // On 256rfr2, BOD is automatically disabled in deep sleep, but
    // some other MCUs need explicit disabling. This should happen shortly
    // before actually sleeping. It's always automatically re-enabled.
    sleep_bod_disable();
    #endif
    sei();
    // AVR guarantees that the instruction after sei is executed, so
    // there is no race condition here
    sleep_cpu();
    // Immediately disable interrupts again, to ensure that
    // exactly one interrupt routine runs after wakeup, so
    // we prevent race conditions and can properly detect if
    // another interrupt than overflow occurred.
    cli();
    if (!timer_match && interruptible) {
      // We were woken up, but the overflow interrupt
      // didn't run, so another interrupt must have
      // triggered. Note that if the overflow
      // interrupt did run, but also another (lower
      // priority) interrupt occured, its flag will
      // remain set and it will immediately wake us up
      // on the next sleep attempt.
      return false;
    }
    // See if overflow happened. Also check the TOV2 flag,
    // for the case where the overflow happens together with
    // another (higher priority) interrupt.
    if (timer_match || TIFR2 & (1 << TOV2)) {
      TIFR2 = (1 << TOV2);
      timer_match = false;
      return true;
    }
  }
}

void SleepHandler::doSleep(uint32_t ms, bool interruptible) {
  // First, make sure we configure timer2 and get it running as
  // fast as possible. This makes the total sleep delay as
  // accurate as possible.

  // Backup timer2 settings
  uint8_t timsk2 = TIMSK2;
  uint8_t tccr2a = TCCR2A;
  uint8_t tccr2b = TCCR2B;

  // Disable timer2 interrupts, stop the timer and reset it
  // to normal mode. It seems that for some reason, switching to
  // asynchronous mode without these registers cleared can somehow
  // mess up the OCR2x register values...
  TIMSK2 = 0;
  TCCR2B = 0;
  TCCR2A = 0;

  // Backup the counter. There is a small race condition here
  // where a counter interrupt could be missed, but since we're
  // about to sleep for a while and timer2 probably isn't used by
  // anything else, that's ok.
  uint8_t tcnt2 = TCNT2;

  // Enable asynchronous mode for timer2 (uses external 32kHz
  // crystal. This might corrupt the settings registers, so we'll
  // have to (re-)set them all.
  // TCCR2B.
  uint8_t assr = ASSR;
  ASSR |= (1 << AS2);

  // Outputs disconnected, normal mode
  TCCR2A = 0;//(1 << WGM21) | (1 << WGM20);

  // Count all the way up to 0xff
  TCNT2 = 0;
  OCR2B = 0xff;

  // Clear any pending interrupt
  TIFR2 = (1 << OCF2B);

  // Start timer, prescaler 1024, meaning a single timer increment
  // is 1/32s
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);

  // Now that timer2 is running, perform any additional power
  // saving preparations

  // Disable Analag comparator
  uint8_t acsr = ACSR;
  ACSR = (1 << ACD);

  // Disable ADC
  uint8_t adcsra = ADCSRA;
  ADCSRA &= ~(1 << ADEN);
  // TODO: When re-enabling the ADC, wait for the AVDDOK bit

  // Disable the TX side of UART0. If the 16u2 it connects to is
  // powered off, it offers a path to ground, so keeping the pin
  // enabled and high (TTL idle) wastes around 1mA of current. To
  // detect if the 16u2 is powered on, see if it pulls the RX pin
  // high (not 100% reliable, but worst case we'll have extra
  // power usage).
  uint8_t ucsr0b = UCSR0B;
  if (UCSR0B & (1 << TXEN0) && !digitalRead(RX0))
    UCSR0B &= ~(1 << TXEN0);

  // Power save mode disables the main clock, but keeps the
  // external clock for timer2 enabled
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();

  cli();

  // Enable the COMPB interrupt to wake us from sleep
  TIMSK2 |= (1 << OCIE2B);

  while (ms >= TIMER_MAX_MS) {
    // Sleep for a complete timer cycle. If interrupt is
    // false, this will always wait for a full cycle and
    // return true. If interrupt is true, this can return
    // false when another interrupt occurs.
    if (sleepUntilMatch(interruptible)) {
      ms -= TIMER_MAX_MS;
      sleepMillis += TIMER_MAX_MS;
    } else {
      // Another interrupt occurred, bail out
      ms = 0;
    }
  }

  OCR2B = MS_TO_TICKS(ms);
  while (ASSR & (1 << OCR2BUB)) /* nothing */;

  // If there's sleep time left, wait for the final interrupt
  if (TCNT2 < OCR2B)
    sleepUntilMatch(interruptible);

  // Stop the counter. Waiting for this write to be completed has
  // the side effect that the TCNT2 is also safe to read now
  // (which contains an invalid value shortly after waking up from
  // sleep).
  TCCR2B = 0;
  while (ASSR & (1 << TCR2BUB)) /* nothing */;

  sleepMillis += TICKS_TO_MS(TCNT2);
  sleep_disable();

  // Clear any pending timer2 interrupts before enabling
  // interrupts globally, just in case
  TIFR2 = 0xff;
  sei();

  // Restore timer2 settings.
  ASSR = assr;
  TCNT2 = tcnt2;
  TCCR2B = tccr2b;
  TCCR2A = tccr2a;
  TIMSK2 = timsk2;

  // Restore other settings
  UCSR0B = ucsr0b;
  ACSR = acsr;
  ADCSRA = adcsra;
  while (ADCSRB & (1 << AVDDOK)) /* nothing */;

  // TODO: Verify precise timings. The datasheet says that the
  // compare match interrupt happens the timer tick after the
  // match, but the asynchronous section also suggests (another?)
  // tick delay. Since a timer tick is fairly long (31ms), this
  // should be measured.
}
