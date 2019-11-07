#include "adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/// Start the ADC module.
void adc_enable()
{
    ADCSRA |= (1 << ADEN);
}

/// Disable the ADC module.
void adc_disable()
{
    ADCSRA &= ~(1 << ADEN);
}

/// Set reference voltage.
/// 0: AREF with Vref turned off
/// 1: AVcc with external capacitor at AREF pin
/// 2: Reserved
/// 3: Internal 1.1 V reference with external capacitor at AREF pin
void adc_set_reference_voltage(uint8_t reference)
{
    ADMUX &= ~((1 << REFS1) | (1 << REFS0));
    ADMUX |= reference << REFS0;  // Both bits are set here
}

/// Reading should left align in ADCH and ADCL.
void adc_set_alignment(uint8_t alignment)
{
    ADMUX &= ~(1 << ADLAR);
    ADMUX |= (alignment << ADLAR);
}

/// Select a channel to read from.
void adc_select_channel(uint8_t channel)
{
    ADMUX &= 0b11110000;
    ADMUX |= 0b00001111 & channel;
}

/// Set how many clock cycles per ADC clock
void adc_set_prescaler(uint8_t factor)
{
    ADCSRA &= 0b11111000;
    ADCSRA |= 0b00000111 & factor;
}

/// Signal to the ADC to start conversion on the selected channel.
void adc_start()
{
    ADCSRA |= (1 << ADSC);
}

/// Hang for ADC conversion to be complete.
void adc_wait()
{
    while (ADCSRA & (1 << ADSC));
}

/// Read only the top register for 8-bit resolution
uint8_t adc_read_left(uint8_t channel)
{
    adc_select_channel(channel);
    adc_start();
    adc_wait();
    return ADCH;
}

/// Read right aligned result for 10-bit resolution
uint16_t adc_read_right(uint8_t channel)
{
    adc_select_channel(channel);
    adc_start();
    adc_wait();
    return ((0b00000111 & ADCH) << 8) | ADCL;
}
