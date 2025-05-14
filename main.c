#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>  // for itoa()

#define TRIG_PIN PD2
#define ECHO_PIN PD3

void uart_init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    // Enable transmitter
    UCSR0B = (1 << TXEN0);
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send_char(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_send_string(const char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

void ultrasonic_init() {
    DDRD |= (1 << TRIG_PIN);   // TRIG output
    DDRD &= ~(1 << ECHO_PIN);  // ECHO input
}

void ultrasonic_trigger() {
    PORTD &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTD |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1 << TRIG_PIN);
}

uint16_t ultrasonic_read_cm() {
    uint32_t count = 0;
    uint16_t distance;

    ultrasonic_trigger();

    while (!(PIND & (1 << ECHO_PIN)));  // wait for ECHO high

    while (PIND & (1 << ECHO_PIN)) {
        _delay_us(1);
        count++;
        if (count > 30000) break;
    }

    distance = (uint16_t)(count / 58.0);
    return distance;
}

int main(void) {
    char buffer[10];
    uart_init(103);  // 9600 baud for 16MHz (UBRR = 103)
    ultrasonic_init();

    while (1) {
        uint16_t distance = ultrasonic_read_cm();
        itoa(distance, buffer, 10);
        uart_send_string("Distance: ");
        uart_send_string(buffer);
        uart_send_string(" cm\r\n");
        _delay_ms(500);
    }
}
