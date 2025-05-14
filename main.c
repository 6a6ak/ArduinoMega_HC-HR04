#define F_CPU 16000000UL  // 16MHz clock speed
#include <avr/io.h> //Rigister sssumption like DDRx, PORTx, PINx
#include <util/delay.h> // For delay functions
#include <stdlib.h> // For itoa function convert integer to string

// UART config
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD - 1)

// HC-SR04 pins on Arduino Mega in Microcontroler ATMega 2560 placed on PORT E & not on PORT D
#define TRIG_PIN PE4  // pin 2 -> Trigger pin -d2-> PORT E 4th bit 
#define ECHO_PIN PE5  // pin 3 -> Echo pin -d3-> PORT E 5th bit

void uart_init(unsigned int ubrr) { // Initialize UART
    UBRR0H = (unsigned char)(ubrr >> 8); // Set baud rate
    UBRR0L = (unsigned char)ubrr; // Set baud rate
    UCSR0B = (1 << TXEN0); // Enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set frame format: 8 data bits, no parity, 1 stop bit
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
    // Send 10µs pulse to trigger
    PORTE &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTE |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTE &= ~(1 << TRIG_PIN);

    // Wait for ECHO to go HIGH
    uint32_t timeout = 0;
    while (!(PINE & (1 << ECHO_PIN))) {
        if (++timeout > 60000) return 0; // timeout safety
    }

    // Start timer (Timer1, Prescaler 8)
    TCCR1A = 0;
    TCCR1B = (1 << CS11); // Prescaler 8
    TCNT1 = 0;

    // Wait for ECHO to go LOW
    timeout = 0;
    while (PINE & (1 << ECHO_PIN)) {
        if (++timeout > 60000) break;
    }

    // Stop timer
    TCCR1B = 0;

    uint16_t ticks = TCNT1;
    // Each tick = 0.5us (16MHz / 8 prescaler)
    uint32_t duration_us = ticks / 2;

    // Convert to cm (distance = (time in us) * 0.034 / 2)
    return (uint16_t)(duration_us * 0.034 / 2);
}

int main(void) {
    // Configure TRIG as output, ECHO as input
    DDRE |= (1 << TRIG_PIN);
    DDRE &= ~(1 << ECHO_PIN);

    // Initialize UART
    uart_init(MYUBRR);

    // Buffer for printing
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
