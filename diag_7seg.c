/* diag_7seg.c
   Diagnostic for 4-digit 7-segment displays.
   - Assumes segments connected to PORTC (PC0..PC6 = a..g)
   - Assumes digit enables connected to PORTB (PB0..PB3)
   If your wiring differs, note which segment lights for each phase.
*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// UART for debug messages
static inline void uart_init(void) {
    uint16_t ubrr = (F_CPU/16/9600 - 1);
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
static inline void uart_tx(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}
static void uart_print(const char *s) {
    while (*s) uart_tx(*s++);
}

// mapping: bit0=a ... bit6=g, bit7=dp
const uint8_t seg_all = 0x7F;

int main(void) {
    // configure ports
    DDRC = 0xFF; // PC0..PC7 outputs for segments
    DDRB |= 0x0F; // PB0..PB3 outputs for digit enables
    PORTC = 0x00; // segments off
    PORTB |= 0x0F; // disable digits (active LOW assumed)

    uart_init();
    uart_print("7seg diag start\r\n");

    // Phase A: light all segments on each digit (assume common-cathode)
    uart_print("Phase A: all segments (common-cathode)\r\n");
    for (uint8_t d=0; d<4; ++d) {
        PORTC = seg_all; // segments on
        PORTB &= ~(1<<d); // enable digit (active LOW)
        _delay_ms(700);
        PORTB |= (1<<d);
        _delay_ms(200);
    }

    _delay_ms(300);

    // Phase B: single segment at a time across digits
    uart_print("Phase B: segments a..g\r\n");
    for (uint8_t seg=0; seg<7; ++seg) {
        uint8_t pat = (1<<seg);
        for (uint8_t d=0; d<4; ++d) {
            PORTC = pat;
            PORTB &= ~(1<<d);
            _delay_ms(400);
            PORTB |= (1<<d);
            _delay_ms(80);
        }
    }

    _delay_ms(300);

    // Phase C: Try common-anode polarity (invert segments and active HIGH digits)
    uart_print("Phase C: try common-anode (inverted)\r\n");
    for (uint8_t d=0; d<4; ++d) {
        PORTC = ~seg_all;
        PORTB |= (1<<d); // enable digit (active HIGH for this test)
        _delay_ms(700);
        PORTB &= ~(1<<d);
        _delay_ms(200);
    }

    uart_print("Diag done - observe segments/digits and report\r\n");

    while (1) {
        _delay_ms(1000);
    }
}
