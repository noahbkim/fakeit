#ifndef FAKEIT_ADC_H
#define FAKEIT_ADC_H

#include <stdint.h>

#define ADC_REFERENCE_AREF 0b00
#define ADC_REFERENCE_AVCC 0b01
#define ADC_REFERENCE_INTERNAL 0b11

#define ADC_PRESCALER_2 0b000
#define ADC_PRESCALER_4 0b010
#define ADC_PRESCALER_8 0b011
#define ADC_PRESCALER_16 0b100
#define ADC_PRESCALER_32 0b101
#define ADC_PRESCALER_64 0b110
#define ADC_PRESCALER_128 0b111

#define ADC_ALIGNMENT_LEFT 0b1
#define ADC_ALIGNMENT_RIGHT 0b0

void adc_enable();
void adc_disable();
void adc_set_reference_voltage(uint8_t reference);
void adc_set_alignment(uint8_t alignment);
void adc_set_prescaler(uint8_t factor);

void adc_select_channel(uint8_t channel);
uint8_t adc_read_high(uint8_t channel);
uint16_t adc_read(uint8_t channel);

#endif //FAKEIT_ADC_H

// Example
// void adc_setup()
// {
//     adc_set_reference_voltage(ADC_REFERENCE_AVCC);  // AVcc
//     adc_set_prescaler(ADC_PRESCALER_128);           // 128 cycles per ADC clock cycle
//     adc_set_alignment(ADC_ALIGNMENT_LEFT);          // We'll only read top 8 bits
//     adc_enable();                                   // Start
// }
