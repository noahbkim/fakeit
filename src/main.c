#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "serial.h"

serial_t SERIAL0 = { &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0 };
serial_t* const S0 = &SERIAL0;

ISR(USART_UDRE_vect)
{
    serial_on_empty_interrupt(S0);
}

int main()
{
    DDRB &= ~(1 << PB0);
    PORTB &= ~(1 << PB0);

    serial_construct(S0, 9600, SERIAL_8N1);
//    sei();

    _delay_ms(1000);

    serial_write(S0, 104);
    serial_write(S0, 105);
    serial_write(S0, 33);
    serial_write(S0, 10);
    serial_destroy(S0);

    PORTB |= (1 << PB0);
    return 0;
}
