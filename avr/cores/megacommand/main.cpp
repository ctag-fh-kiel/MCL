#define IS_ISR_ROUTINE

#include <Arduino.h>
#include <WProgram.h>
#include <wiring_private.h>
//#include <Events.h>
extern "C" {
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
}

#include "MCLSeq.h"

#define OLED_CLK 52
#define OLED_MOSI 51

// Used for software or hardware SPI
#define OLED_CS 42
#define OLED_DC 44
#define OLED_RESET 38

#ifdef OLED_DISPLAY
Adafruit_SSD1305 oled_display(OLED_DC, OLED_RESET, OLED_CS);
#endif
volatile uint8_t MidiUartParent::handle_midi_lock = 0;

void my_init_ram(void) __attribute__((naked)) __attribute__((used))
__attribute__((section(".init3")));

void(* hardwareReset) (void) = 0;

void my_init_ram(void) {
// Set PL6 as output

#ifdef MEGACOMMAND
  DDRL |= _BV(PL6);
  PORTL &= ~(_BV(PL6));
#else
  DDRB |= _BV(PB0);
  PORTB &= ~(_BV(PB0));
#endif
  XMCRA |= _BV(SRE);

  //Leds

  DDRE |= _BV(PE4) | _BV(PE5);
  return;
  //SRAM tests

  volatile uint8_t *ptr;
  uint8_t linear = 0;
  volatile uint8_t read_bank0, read_bank1 = 0;

  for (ptr = reinterpret_cast<uint8_t *> (0x2200); ptr < reinterpret_cast<uint8_t *> (0xFFFF); ptr++) {
   switch_ram_bank(0);
   uint8_t random_number = pgm_read_byte(rand_ptr++) || 1;
   //Store a random number in bank 0
   *ptr = random_number;
   //Read value back from bank 0
   read_bank0 = *ptr;
   switch_ram_bank(1);
   //Store same random number in bank 1
   *ptr = read_bank0;
   //Read bank1
   read_bank1 = *ptr;
   if ((read_bank0 == 0) || (read_bank1 == 0) || (read_bank0 != random_number) || (read_bank1 != random_number))  {
     goto fail;
   }
   // Check that the RAM bank toggle actually toggles to a different bank.

   random_number++;
   //Store new random number in to bank1
   *ptr = random_number;
   //Read bank1
   read_bank1 = *ptr;
   //Read bank0;
   switch_ram_bank(0);
   read_bank0 = *ptr;
   if (read_bank0 == read_bank1) {
     goto fail;
   }
   //Store lineearly increases values in bank 1, for linear read later. (loop compaction)
   *ptr = linear++;
  }
  switch_ram_bank(0);
  //Linear read.
  linear = 0;
  for (ptr = reinterpret_cast<uint8_t *> (0x2200); ptr < reinterpret_cast<uint8_t *> (0xFFFF); ptr++) {
    read_bank0 = *ptr;
    if (read_bank0 != linear) {
      goto fail;
    }
    linear++;
  }
  switch_ram_bank(1);
  return;

  fail:
  while (1) {
     setLed();
     setLed2();
  }
}
uint8_t tcnt2;

void timer_init(void) {
  TCCR1A = _BV(WGM10);              //  | _BV(COM1A1) | _BV(COM1B1);
  TCCR1B |= _BV(CS10) | _BV(WGM12); // every cycle

  // http://www.arduinoslovakia.eu/application/timer-calculator
  // Microcontroller: ATmega2560
  // Created: 2017-10-28T08:18:15.310Z

  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // 5000 Hz (16000000/((49+1)*64))
  OCR1A = 49;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 64
  TCCR1B |= (1 << CS11) | (1 << CS10);
// Output Compare Match A Interrupt Enable
#ifdef MEGACOMMAND
  TIMSK1 |= (1 << OCIE1A);
#else
  TIMSK |= (1 << OCIE1A);
#endif
  // TCCR2A = _BV(WGM20) | _BV(WGM21) | _BV(CS20) | _BV(CS21); // ) | _BV(CS21);
  // // | _BV(COM21);

  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0;
  // 1000 Hz (16000000/((249+1)*64))
  OCR3A = 249;
  // CTC
  TCCR3B |= (1 << WGM32);
  // Prescaler 64
  TCCR3B |= (1 << CS31) | (1 << CS30);
// Output Compare Match A Interrupt Enable
#ifdef MEGACOMMAND
  TIMSK3 |= (1 << OCIE3A);
#else
  ETIMSK |= (1 << OCIE3A);
#endif
}

void init(void) {
  /** Disable watchdog. **/
  wdt_disable();
  //  wdt_enable(WDTO_15MS);

  // Configure Port C as 8 channels of output. disable pullup resistors.
  DDRC = 0xFF;
  PORTC = 0x00;

  /* move interrupts to bootloader section */
  MCUCR = _BV(IVCE);

  // Enable External SRAM
  MCUCR = _BV(SRE);

  // activate lever converter
  SET_BIT(DDRD, PD4);
  SET_BIT(PORTD, PD4);


  //For MC SMD. Level shifter 1 + 2 enable.
  //PL4 == MEGA2560 level shifter enable
  //PL3 == Atmega16/34 level shifter enable
  //Only one should be active at any time.

  DDRL |= _BV(PL4) | _BV(PL3);

  LOCAL_SPI_ENABLE();

  timer_init();

}

void setup();
void loop();

ISR(TIMER1_COMPA_vect) {

  select_bank(0);

  clock++;
  MidiClock.div192th_countdown++;
  if (MidiClock.state == 2) {
    if (MidiClock.div192th_countdown >= MidiClock.div192_time) {
      if (MidiClock.div192th_counter != MidiClock.div192th_counter_last) {
        MidiClock.increment192Counter();
        MidiClock.div192th_countdown = 0;
        MidiClock.div192th_counter_last = MidiClock.div192th_counter;
        if (MidiClock.inCallback) { return; }
        MidiClock.inCallback = true;
        uint8_t _midi_lock_tmp = MidiUartParent::handle_midi_lock;
        MidiUartParent::handle_midi_lock = 1;
        sei();
        mcl_seq.seq();
        MidiUartParent::handle_midi_lock = _midi_lock_tmp;
        MidiClock.inCallback = false;
      }
    }
  }

  if (!MidiUartParent::handle_midi_lock)  {
   uint8_t _irqlock_tmp = SREG;
   MidiUartParent::handle_midi_lock = 1;
   sei();
   handleIncomingMidi();
   SREG = _irqlock_tmp;
   MidiUartParent::handle_midi_lock = 0;
  }
}

// XXX CMP to have better time

static uint16_t oldsr = 0;
volatile uint8_t *rand_ptr = 0;
uint16_t minuteclock = 0;

ALWAYS_INLINE() void gui_poll() {
  static bool inGui = false;
  if (inGui) {
    return;
  }

  inGui = true;

  uint16_t sr = SR165.read16();
  if (sr != oldsr) {
    Buttons.clear();
    Buttons.poll(sr >> 8);
    Encoders.poll(sr);
    oldsr = sr;
    pollEventGUI();
  }
  inGui = false;
}


ISR(TIMER3_COMPA_vect) {

  select_bank(0);

  slowclock++;
  minuteclock++;

  if (minuteclock == 60000) {
    minuteclock = 0;
    clock_minutes++;
  }

  MidiUart.tickActiveSense();
  MidiUart2.tickActiveSense();
  MidiUartUSB.tickActiveSense();

  sei();
  gui_poll();
}

void handleIncomingMidi() {

  Midi.processSysex();
  Midi2.processSysex();
  MidiUSB.processSysex();
  Midi.processMidi();
  Midi2.processMidi();
  MidiUSB.processMidi();

}

void change_usb_mode(uint8_t mode) {
  uint8_t change_mode_msg[] = {0xF0, 0x7D, 0x4D, 0x43, 0x4C, 0x01, mode, 0xF7};
  MidiUartUSB.m_putc(change_mode_msg, sizeof(change_mode_msg));
  delay(200);
  if (mode == USB_SERIAL) {
     MidiUartUSB.mode = UART_SERIAL; MidiUartUSB.set_speed(SERIAL_SPEED);
  }
}

int main(void) {


  //PK0 PK1 PK2 are MegaCMD control lines to the Atmega32u2
  //Not available on the DIY version.
  PORTK = 0x00;
  PORTK |= _BV(PK0) | _BV(PK1) | _BV(PK2); //enable pullup or set high

  DDRK = 0x00;
  DDRK |= _BV(PK1) | _BV(PK0); //set output


  //Screen + SD card may need some time to 'charge' before interfacing.
  delay(100);
  init();

  uint16_t sr = SR165.read16();
  Buttons.clear();
  Buttons.poll(sr >> 8);
  Encoders.poll(sr);
  oldsr = sr;

  sei();

  DEBUG_INIT();

  #ifdef RUNNING_STATUS_OUT
  MidiUartUSB.running_status_enabled = false;
  #endif

// Set SD card select HIGH before initialising OLED.
#ifdef MEGACOMMAND
  PORTB |= (1 << PB0);
#else
  PORTE |= (1 << PE7);
#endif
  setup();
  for (;;) {
    loop();
    GUI.loop();
  }
  return 0;
}

