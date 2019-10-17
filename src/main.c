#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

int main() {
    DDRD |= (1 << DDD0);
    while (true) {
        PORTD |= (1 << PD0);
        _delay_ms(1000);
        PORTD &= ~(1 << PD0);
        _delay_ms(1000);
    }
}
