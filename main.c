#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD - 1)

// --- Mega pins ---
#define TRIG_PIN PE4  // Arduino pin 2
#define ECHO_PIN PE5  // Arduino pin 3

void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_print(const char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

uint16_t read_distance_cm() {
    PORTE &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTE |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTE &= ~(1 << TRIG_PIN);

    while (!(PINE & (1 << ECHO_PIN)));

    uint32_t count = 0;
    while (PINE & (1 << ECHO_PIN)) {
        _delay_us(1);
        count++;
        if (count > 30000) break;
    }

    return (uint16_t)(count * 0.034 / 2);
}

int main(void) {
    DDRE |= (1 << TRIG_PIN);
    DDRE &= ~(1 << ECHO_PIN);

    uart_init(MYUBRR);

    char buffer[16];
    uint16_t distance;

    while (1) {
        distance = read_distance_cm();
        itoa(distance, buffer, 10);
        uart_print("Distance: ");
        uart_print(buffer);
        uart_print(" cm\r\n");
        _delay_ms(500);
    }
}
