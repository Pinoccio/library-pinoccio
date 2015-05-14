#include <Arduino.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include "SleepHandler.h"

static volatile bool timer_match;

Duration SleepHandler::totalSleep = {0, 0};
Pbbe::LogicalPin::mask_t SleepHandler::pinWakeups = 0;

ISR(SCNT_CMP3_vect) {
  timer_match = true;
}

/* Do nothing, just meant to wake up the Scout. We would want to declare
 * this ISR weak, so a custom sketch can still use it - but it seems
 * doing so prevents the empty ISR from being used (instead falling bad
 * to the also weak __bad_interrupt). */
EMPTY_INTERRUPT(PCINT0_vect);

uint32_t SleepHandler::read_sccnt() {
  // Read LL first, that will freeze the other registers for reading
  uint32_t sccnt = SCCNTLL;
  sccnt |= (uint32_t)SCCNTLH << 8;
  sccnt |= (uint32_t)SCCNTHL << 16;
  sccnt |= (uint32_t)SCCNTHH << 24;
  return sccnt;
}

uint32_t SleepHandler::read_scocr3() {
  // Read LL first, that will freeze the other registers for reading
  uint32_t sccnt = SCOCR3LL;
  sccnt |= (uint32_t)SCOCR3LH << 8;
  sccnt |= (uint32_t)SCOCR3HL << 16;
  sccnt |= (uint32_t)SCOCR3HH << 24;
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
  // Timer2 is used for PWM on the blue led on the Pinoccio scout, by
  // default using 16Mhz/64=250kHz. We set the prescaler to 1 (32kiHz)
  // to come as close to that as possible. This results in a PWM
  // frequency of 32k/256 = 128Hz, which should still be sufficient for
  // a LED.
  TCCR2B = (TCCR2B & ~((1<<CS22)|(1<<CS21))) | (1<<CS20);

  // Enable the pin change interrupt 0 (for PCINT0-7). Individual pins
  // remain disabled until we actually sleep.
  PCICR |= (1 << PCIE0);
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
      // interrupt did trigger but not run yet, but also another
      // (lower priority) interrupt occured, its flag will
      // remain set and it will immediately wake us up
      // on the next sleep attempt.
      return false;
    }
    // See if overflow happened. Also check the IRQSCP3 flag,
    // for the case where the overflow happens together with
    // another (higher priority) interrupt.
    if (timer_match || SCIRQS & (1 << IRQSCP3)) {
      SCIRQS = (1 << IRQSCP3);
      timer_match = true;
      return true;
    }
  }
}

void SleepHandler::scheduleSleep(uint32_t ms) {
  uint32_t ticks = msToTicks(ms);
  // Make sure we cannot "miss" the compare match if a low timeout is
  // passed (really only ms = 0, which is forbidden, but handle it
  // anyway).
  if (ticks < 2) ticks = 2;
  // Disable interrupts to prevent the counter passing the target before
  // we clear the IRQSCP3 flag (due to other interrupts happening)
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Schedule SCNT_CMP3 when the given counter is reached
    write_scocr3(read_sccnt() + ticks);

    // Clear any previously pending interrupt
    SCIRQS = (1 << IRQSCP3);
    timer_match = false;
  }
}

uint32_t SleepHandler::scheduledTicksLeft() {
  uint32_t left = read_scocr3() - read_sccnt();

  // If a compare match has occured, we're past the end of sleep already.
  // We check this _after_ grabbing the counter value above, to prevent
  // a race condition where the counter goes past the compare value
  // after checking for the timer_match flag. We check both the
  // interrupt flag and the timer_match flag, to handle both before
  // and after sleeping (since before sleeping, the IRQ is disabled, but
  // during sleep the wakeup clears the flag).
  if ((SCIRQS & (1 << IRQSCP3)) || timer_match)
    return 0;
  return left;
}

void SleepHandler::doSleep(bool interruptible) {
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

  uint8_t eimsk = EIMSK;
  uint8_t pcmsk0 = PCMSK0;

  // Clear any pending PCINT0 flag
  PCIFR |= (1 << PCIF0);

  // TODO: Replace this hardcoded list with something based on
  // digitalPinToPCICR / digitalPinToInterrupt (after they've been
  // fixed for pinoccio). The challenge there is to make the code not
  // crazily slow by calling those macros for all possible pins in a
  // loop.

  // First, enable pins for the pin change interrupt. The interrupt
  // itself was already enabled in setup().
  if (Pbbe::LogicalPin(SS).in(pinWakeups))
    PCMSK0 |= (1 << PCINT0);
  if (Pbbe::LogicalPin(SCK).in(pinWakeups))
    PCMSK0 |= (1 << PCINT1);
  if (Pbbe::LogicalPin(MOSI).in(pinWakeups))
    PCMSK0 |= (1 << PCINT2);
  if (Pbbe::LogicalPin(MISO).in(pinWakeups))
    PCMSK0 |= (1 << PCINT3);
  // PCINT4/5/6 are the RGB led
  if (Pbbe::LogicalPin(D2).in(pinWakeups))
    PCMSK0 |= (1 << PCINT7);
  // PCINT8 is RX0, PCINT9-23 are not connected on 256rfr2

  // Then do the external interrupts. INT0-3 can detect edges during
  // sleep, but INT4-7 can only detect low-level during sleep. This
  // means that a low level on those pins will wake us from sleep
  // immediately.
  // We pass in NULL for the callback function, since we don't really
  // care about the interrupt itself.
  if (Pbbe::LogicalPin(SCL).in(pinWakeups))
    attachInterrupt(2, NULL, CHANGE); // INT0
  if (Pbbe::LogicalPin(SDA).in(pinWakeups))
    attachInterrupt(3, NULL, CHANGE); // INT1
  if (Pbbe::LogicalPin(RX1).in(pinWakeups))
    attachInterrupt(4, NULL, CHANGE); // INT2
  if (Pbbe::LogicalPin(TX1).in(pinWakeups))
    attachInterrupt(5, NULL, CHANGE); // INT3
  if (Pbbe::LogicalPin(D4).in(pinWakeups))
    attachInterrupt(0, NULL, LOW); // INT4
  if (Pbbe::LogicalPin(D5).in(pinWakeups))
    attachInterrupt(1, NULL, LOW); // INT5
  if (Pbbe::LogicalPin(D7).in(pinWakeups))
    attachInterrupt(6, NULL, LOW); // INT6
  if (Pbbe::LogicalPin(BATT_ALERT).in(pinWakeups))
    attachInterrupt(7, NULL, LOW); // INT7

  // Check to see if we haven't passed the until time yet. Due to
  // wraparound, we convert to an integer, but this effectively halves
  // the maximum sleep duration (if until is > 2^31 from from now, it'll
  // look like the same as if until is in the past).
  if (!(SCIRQS & (1 << IRQSCP3))) {
    // Enable the SCNT_CMP3 interrupt to wake us from sleep
    SCIRQM |= (1 << IRQMCP3);

    uint32_t before = read_sccnt();
    sleepUntilMatch(interruptible);
    uint32_t after = read_sccnt();
    totalSleep += (uint64_t)(after - before) * US_PER_TICK;

    // Disable the SCNT_CMP3 interrupt again
    SCIRQM &= ~(1 << IRQMCP3);
  }

  sleep_disable();

  PCMSK0 = pcmsk0;
  // Instead of calling detachInterrupt a dozen times, just restore the
  // original external interrupt mask
  EIMSK = eimsk;

  sei();

  // Restart timer2
  TCCR2B = tccr2b;

  // Restore other settings
  UCSR0B = ucsr0b;
  ACSR = acsr;
  ADCSRA = adcsra;
  while (!(ADCSRB & (1 << AVDDOK))) /* nothing */;
}

void SleepHandler::setPinWakeup(uint8_t pin, bool enable) {
  if (enable)
    pinWakeups |= Pbbe::LogicalPin(pin).mask();
  else
    pinWakeups &= ~Pbbe::LogicalPin(pin).mask();
}

bool SleepHandler::pinWakeupSupported(uint8_t pin) {
  Pbbe::LogicalPin::mask_t supported =
    Pbbe::LogicalPin(D2).mask() |
    Pbbe::LogicalPin(D4).mask() |
    Pbbe::LogicalPin(D5).mask() |
    Pbbe::LogicalPin(D7).mask() |
    Pbbe::LogicalPin(BATT_ALERT).mask() |
    Pbbe::LogicalPin(SS).mask() |
    Pbbe::LogicalPin(MOSI).mask() |
    Pbbe::LogicalPin(MISO).mask() |
    Pbbe::LogicalPin(SCK).mask() |
    Pbbe::LogicalPin(TX1).mask() |
    Pbbe::LogicalPin(RX1).mask() |
    Pbbe::LogicalPin(SDA).mask() |
    Pbbe::LogicalPin(SCL).mask();

  return Pbbe::LogicalPin(pin).in(supported);
}
