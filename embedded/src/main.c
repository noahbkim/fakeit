#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "serial.h"

SERIAL_SETUP(0)

void adc_setup()
{
    adc_set_reference_voltage(ADC_REFERENCE_AVCC);  // AVcc
    adc_set_prescaler(ADC_PRESCALER_128);           // 128 cycles per ADC clock cycle
    adc_set_alignment(ADC_ALIGNMENT_LEFT);          // We'll only read top 8 bits
    adc_enable();                                   // Start
}

int main()
{
    adc_setup();
    serial_construct(S0, 9600, SERIAL_8N1);

    uint8_t sample = 0;
    uint8_t debounce = 0;
    while (true)
    {
        sample = adc_read_left(0);
        if (sample > 20 && debounce == 0) {
            serial_write(S0, 107);
            debounce = 10;
        } else if (debounce > 0) {
            --debounce;
        }
    }

    serial_destroy(S0);
    return 0;
}
