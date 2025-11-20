#define F_CPU 16000000UL  // 16MHz clock speed
#include <avr/io.h> // DDRx, PORTx, PINx
#include <util/delay.h> // _delay_us, _delay_ms
#include <stdlib.h> // itoa
#include <stdint.h>

// UART config
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD - 1)

// HC-SR04 pins on Arduino Mega (ATmega2560) - PORT E
#define TRIG_PIN PE4  // Trigger on digital pin 2 -> PORTE bit 4
#define ECHO_PIN PE5  // Echo on digital pin 3 -> PORTE bit 5

void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0); // enable TX only
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

void uart_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_print(const char* str) {
    while (*str) uart_transmit((unsigned char)*str++);
}

// --- 7-segment configuration (change to match your wiring) ---
// Display backends: either TM1637 module or direct 4-digit multiplexed display.
// Use TM1637 driver by default (Wokwi uses a TM1637 module).
#ifndef USE_TM1637
#define USE_TM1637 1
#endif

// Direct 4-digit multiplexed display (default wiring used earlier):
// Segments on PORTC: PC0=a, PC1=b, PC2=c, PC3=d, PC4=e, PC5=f, PC6=g, PC7=dp
// Digit enable on PORTB: PB0..PB3 (active LOW for common-cathode)
#define SEG_PORT PORTC
#define SEG_DDR  DDRC
#define DIG_PORT PORTB
#define DIG_DDR  DDRB

static const uint8_t seg_table[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

void sevenseg_init(void) {
    SEG_DDR = 0xFF;       // segments as outputs
    DIG_DDR |= 0x0F;      // PB0..PB3 outputs for digit enables
    SEG_PORT = 0x00;      // segments off
    DIG_PORT |= 0x0F;     // disable all digits (active LOW)
}

// value 0..9999, ms total display time
void display_number(uint16_t value, uint16_t ms) {
    uint8_t digits[4] = {0,0,0,0};
    digits[0] = (value / 1000) % 10;
    digits[1] = (value / 100) % 10;
    digits[2] = (value / 10) % 10;
    digits[3] = value % 10;

    uint16_t elapsed = 0;
    while (elapsed < ms) {
        for (uint8_t i = 0; i < 4; ++i) {
            // output segments (common-cathode - HIGH lights segment)
            SEG_PORT = seg_table[digits[i]];
            // enable digit i (active LOW)
            DIG_PORT &= ~(1 << i);
            _delay_ms(2);
            // disable digit
            DIG_PORT |= (1 << i);
        }
        elapsed += 8; // approx 4*2ms
    }
}

// TM1637 driver (used in Wokwi diagram)
// Connections (from diagram.json): DIO -> D6, CLK -> D7
#if USE_TM1637
#define TM_DIO PH3
#define TM_CLK PH4
#define TM_PORT PORTH
#define TM_DDR  DDRH
#define TM_PIN  PINH

static const uint8_t tm_digits[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

static inline void tm_delay(void) { _delay_us(50); }

static void tm_init_pins(void) {
    // Set CLK as output high, release DIO (input with pull-up)
    TM_DDR |= (1 << TM_CLK);
    TM_PORT |= (1 << TM_CLK);
    TM_DDR &= ~(1 << TM_DIO); // input
    TM_PORT |= (1 << TM_DIO); // enable pull-up
}

static void tm_start(void) {
    // CLK high, DIO high
    TM_PORT |= (1 << TM_DIO);
    TM_PORT |= (1 << TM_CLK);
    tm_delay();
    // DIO low while CLK high
    TM_DDR |= (1 << TM_DIO);
    TM_PORT &= ~(1 << TM_DIO);
    tm_delay();
    TM_PORT &= ~(1 << TM_CLK);
    tm_delay();
}

static void tm_stop(void) {
    TM_PORT &= ~(1 << TM_CLK);
    TM_DDR |= (1 << TM_DIO);
    TM_PORT &= ~(1 << TM_DIO);
    tm_delay();
    TM_PORT |= (1 << TM_CLK);
    tm_delay();
    TM_DDR &= ~(1 << TM_DIO); // release DIO
    TM_PORT |= (1 << TM_DIO);
    tm_delay();
}

static void tm_write_byte(uint8_t b) {
    for (uint8_t i = 0; i < 8; ++i) {
        TM_PORT &= ~(1 << TM_CLK);
        if (b & 0x01) {
            // send 1: release DIO (input with pull-up)
            TM_DDR &= ~(1 << TM_DIO);
            TM_PORT |= (1 << TM_DIO);
        } else {
            // send 0: drive DIO low
            TM_DDR |= (1 << TM_DIO);
            TM_PORT &= ~(1 << TM_DIO);
        }
        tm_delay();
        TM_PORT |= (1 << TM_CLK);
        tm_delay();
        b >>= 1;
    }

    // ack bit
    TM_PORT &= ~(1 << TM_CLK);
    TM_DDR &= ~(1 << TM_DIO); // release DIO
    tm_delay();
    TM_PORT |= (1 << TM_CLK);
    tm_delay();
    // read ack (ignore value)
    TM_PORT &= ~(1 << TM_CLK);
    tm_delay();
}

void tm_display_number(uint16_t value) {
    uint8_t digits[4] = {0,0,0,0};
    digits[0] = (value / 1000) % 10;
    digits[1] = (value / 100) % 10;
    digits[2] = (value / 10) % 10;
    digits[3] = value % 10;

    // data command: 0x40 (normal write)
    tm_start();
    tm_write_byte(0x40);
    tm_stop();
    _delay_us(200);

    // address command: 0xC0
    tm_start();
    tm_write_byte(0xC0);
    for (uint8_t i = 0; i < 4; ++i) {
        tm_write_byte(tm_digits[digits[i]]);
    }
    tm_stop();

    // display control: 0x88 = display on, brightness 0
    _delay_us(200);
    tm_start();
    tm_write_byte(0x88 | 0x07); // max brightness
    tm_stop();
}
#endif

// Calibration offset in cm (set positive if display reads low)
#ifndef CALIB_OFFSET_CM
#define CALIB_OFFSET_CM 0
#endif

// Measure using hardware Timer1 for accurate timing.
// Timer1 runs with prescaler 8 -> 16MHz/8 = 2MHz => tick = 0.5 us
uint16_t read_distance_cm(void) {
    // Trigger pulse
    PORTE &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTE |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTE &= ~(1 << TRIG_PIN);

    // Wait for ECHO high with a software timeout (microseconds)
    uint32_t timeout = 0;
    while (!(PINE & (1 << ECHO_PIN))) {
        _delay_us(1);
        if (++timeout >= 30000u) return 0; // no echo
    }

    // Configure Timer1 and start (prescaler 8)
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TCCR1B |= (1 << CS11); // prescaler = 8

    // Wait for ECHO low or timeout (limit ticks to ~60000)
    while (PINE & (1 << ECHO_PIN)) {
        if (TCNT1 >= 60000u) break;
    }

    // Stop Timer1
    TCCR1B = 0;
    uint16_t ticks = TCNT1; // each tick = 0.5 us

    // Debug: print raw ticks and approximate microseconds
    char dbuf[32];
    ltoa((long)ticks, dbuf, 10);
    uart_print("Raw ticks: "); uart_print(dbuf); uart_print("\r\n");
    // approximate microseconds = ticks * 0.5 -> integer division
    ltoa((long)(ticks / 2), dbuf, 10);
    uart_print("Us (approx): "); uart_print(dbuf); uart_print("\r\n");

    // Convert ticks to cm: us = ticks * 0.5 ; cm = us / 58
    // => cm = ticks / 116  (rounding)
    uint32_t dist_cm = (uint32_t)((ticks + 58) / 116);

    int32_t adj = (int32_t)dist_cm + (int32_t)CALIB_OFFSET_CM;
    if (adj < 0) adj = 0;
    return (uint16_t)adj;
}

int main(void) {
    // Configure TRIG as output, ECHO as input
    DDRE |= (1 << TRIG_PIN);
    DDRE &= ~(1 << ECHO_PIN);
    // make sure TRIG starts low
    PORTE &= ~(1 << TRIG_PIN);

    // Initialize UART
    uart_init(MYUBRR);

    // Debug banner
    uart_print("--- HC-SR04 firmware boot ---\r\n");
    // show pin direction and initial PIN state
    char buf[64];
    itoa((int)DDRE, buf, 10); uart_print("DDRE: "); uart_print(buf); uart_print("\r\n");
    itoa((int)PORTE, buf, 10); uart_print("PORTE: "); uart_print(buf); uart_print("\r\n");

    // Initialize display backend
#if USE_TM1637
    tm_init_pins();
#else
    sevenseg_init();
#endif

    // Buffer for printing
    char buffer[16];
    uint16_t distance;

        while (1) {
            distance = read_distance_cm();
            itoa(distance, buffer, 10);
            uart_print("Distance: ");
            uart_print(buffer);
            uart_print(" cm\r\n");
            // show on selected display backend
#if USE_TM1637
            tm_display_number(distance > 9999 ? 9999 : distance);
            _delay_ms(300);
#else
            display_number(distance > 9999 ? 9999 : distance, 300);
#endif
        }
}
