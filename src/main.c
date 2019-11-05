#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"

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
    return 0;
}
