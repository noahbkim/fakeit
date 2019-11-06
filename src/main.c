#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "serial.h"

serial_t serial = { &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0 };

ISR(UART0_UDRE_vect)
{
    serial_on_empty_interrupt(&serial);
}

int main()
{
    DDRB &= ~(1 << PB0);
    PORTB &= ~(1 << PB0);

    serial_construct(&serial, 9600, SERIAL_8N1);
    _delay_ms(1000);

    serial_write(&serial, 104);
    serial_write(&serial, 105);
    serial_write(&serial, 33);
    serial_write(&serial, 10);
    serial_destroy(&serial);

    PORTB |= (1 << PB0);
    return 0;
}
