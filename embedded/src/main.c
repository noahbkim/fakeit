#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "serial.h"

SERIAL_SETUP(0)

int main()
{
    serial_construct(S0, 9600, SERIAL_8N1);
    for (int i = 0; i < 8; i++)
    {
        _delay_ms(500);
        serial_write(S0, 107);
    }
    serial_destroy(S0);
    return 0;
}
