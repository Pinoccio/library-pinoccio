#include <Arduino.h>
#include <avr/sleep.h>
#include "SleepHandler.h"

static volatile bool timer_match;

ISR(SCNT_CMP3_vect) {
  timer_match = true;
}


uint32_t SleepHandler::read_sccnt() {
  // Read LL first, that will freeze the other registers for reading
  uint32_t sccnt = SCCNTLL;
  sccnt |= (uint32_t)SCCNTLH << 8;
  sccnt |= (uint32_t)SCCNTHL << 16;
  sccnt |= (uint32_t)SCCNTHH << 24;
  return sccnt;
}

void SleepHandler::write_scocr3(uint32_t val) {
  // Write LL last, that will update the entire register atomically
  SCOCR3HH = val >> 24;
  SCOCR3HL = val >> 16;
  SCOCR3LH = val >> 8;
  SCOCR3LL = val;
}

void SleepHandler::setup() {
  // Enable asynchronous mode for timer2. This is required to start the
  // 32kiHz crystal at all, so we can use it for the symbol counter. See
  // http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=142962
  ASSR |= (1 << AS2);

  // Timer2 is used for PWM on the blue led on the Pinoccio scout, by
  // default using 16Mhz/64=250kHz. We set the prescaler to 1 (32kiHz)
  // to come as close to that as possible. This results in a PWM
  // frequency of 32k/256 = 128Hz, which should still be sufficient for
  // a LED.
  TCCR2B = (TCCR2B & ~((1<<CS22)|(1<<CS21))) | (1<<CS20);

  // Enable the symbol counter, using the external 32kHz crystal
  // SCCR1 is left at defaults, with CMP3 in absolute compare mode.
  SCCR0 |= (1 << SCEN) | (1 << SCCKSEL);
}

// Sleep until the timer match interrupt fired. If interruptible is
// true, this can return before if some other interrupt wakes us up
// from sleep. If this happens, true is returned.
bool SleepHandler::sleepUntilMatch(bool interruptible) {
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
    // See if overflow happened. Also check the IRQSCP3 flag,
    // for the case where the overflow happens together with
    // another (higher priority) interrupt.
    if (timer_match || SCIRQS & (1 << IRQSCP3)) {
      SCIRQS = (1 << IRQSCP3);
      timer_match = false;
      return true;
    }
  }
}

void SleepHandler::doSleep(uint32_t until_tick, bool interruptible) {
  // Disable Analag comparator
  uint8_t acsr = ACSR;
  ACSR = (1 << ACD);

  // Disable ADC
  uint8_t adcsra = ADCSRA;
  ADCSRA &= ~(1 << ADEN);

  // Disable the TX side of UART0. If the 16u2 it connects to is
  // powered off, it offers a path to ground, so keeping the pin
  // enabled and high (TTL idle) wastes around 1mA of current. To
  // detect if the 16u2 is powered on, see if it pulls the RX pin
  // high (not 100% reliable, but worst case we'll have extra
  // power usage or some serial garbage).
  // Note that this only takes effect after finishing the current byte,
  // so if transmission is still going on when we sleep, this doesn't
  // actually help.
  // An alternative would be to enable the pullup on the pin disable TX
  // unconditionally, which keeps the line high. However, this still
  // wastes 10μA (nearly doubling the minimum power usage).
  uint8_t ucsr0b = UCSR0B;
  if (UCSR0B & (1 << TXEN0) && !digitalRead(RX0))
    UCSR0B &= ~(1 << TXEN0);

  // Power save mode disables the main clock, but keeps the
  // external clock enabled for timer2 and symbol counter
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();

  cli();

  // Stop timer2, otherwise it will keep running in power-save mode
  uint8_t tccr2b = TCCR2B;
  TCCR2B = 0;

  // Schedule SCNT_CMP3 when the given counter is reached
  write_scocr3(until_tick);

  // Clear any pending interrupt
  SCIRQS = (1 << IRQSCP3);

  // Check to see if we haven't passed the until time yet. Due to
  // wraparound, we convert to an integer, but this effectively halves
  // the maximum sleep duration (if until is > 2^31 from from now, it'll
  // look like the same as if until is in the past).
  if ((int32_t)(until_tick - read_sccnt()) > 0) {
    // Enable the SCNT_CMP3 interrupt to wake us from sleep
    SCIRQM = (1 << IRQMCP3);

    sleepUntilMatch(interruptible);

    // Disable the SCNT_CMP3 interrupt again
    SCIRQM &= ~(1 << IRQMCP3);
  }

  sleep_disable();

  sei();

  // Restart timer2
  TCCR2B = tccr2b;

  // Restore other settings
  UCSR0B = ucsr0b;
  ACSR = acsr;
  ADCSRA = adcsra;
  while (ADCSRB & (1 << AVDDOK)) /* nothing */;
}
