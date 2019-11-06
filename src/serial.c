#include "serial.h"
#include <util/atomic.h>
#include <avr/io.h>
#include <stdbool.h>

// https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/HardwareSerial.h

#define SERIAL_MODE_ASYNCHRONOUS_NORMAL_SCALE 16
#define SERIAL_MODE_ASYNCHRONOUS_DOUBLE_SCALE 8
#define SERIAL_MODE_SYNCHRONOUS_MASTER 2

#define TRANSMIT_ATOMIC ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

inline void serial_set_ubrr(serial_t* serial, uint16_t ubrr)
{
    *serial->ubrrh = ubrr >> 8;
    *serial->ubrrl = ubrr;
}

inline void serial_double_speed_enable(serial_t* serial) { *serial->ucsra |= (1 << U2X0); }
inline void serial_double_speed_disable(serial_t* serial) { *serial->ucsra &= ~(1 << U2X0); }

inline void serial_transmit_enable(serial_t* serial) { *serial->ucsrb |= (1 << TXEN0); }
inline void serial_transmit_disable(serial_t* serial) { *serial->ucsrb &= ~(1 << TXEN0); }
inline void serial_receive_enable(serial_t* serial) { *serial->ucsrb |= (1 << RXEN0); }
inline void serial_receive_disable(serial_t* serial) { *serial->ucsrb &= ~(1 << RXEN0); }

inline void serial_interrupt_transmit_enable(serial_t* serial) { *serial->ucsrb |= (1 << TXCIE0); }
inline void serial_interrupt_transmit_disable(serial_t* serial) { *serial->ucsrb &= ~(1 << TXCIE0); }
inline uint8_t serial_interrupt_transmit_enabled(serial_t* serial) { return *serial->ucsrb & (1 << TXCIE0); }

inline void serial_interrupt_receive_enable(serial_t* serial) { *serial->ucsrb |= (1 << RXCIE0); }
inline void serial_interrupt_receive_disable(serial_t* serial) { *serial->ucsrb &= ~(1 << RXCIE0); }
inline uint8_t serial_interrupt_receive_enabled(serial_t* serial) { return *serial->ucsrb & (1 << RXCIE0); }

inline void serial_interrupt_empty_enable(serial_t* serial) { *serial->ucsrb |= (1 << UDRIE0); }
inline void serial_interrupt_empty_disable(serial_t* serial) { *serial->ucsrb &= ~(1 << UDRIE0); }
inline uint8_t serial_interrupt_empty_enabled(serial_t* serial) { return *serial->ucsrb & (1 << UDRIE0); }

inline uint8_t serial_is_empty(serial_t* serial) { return *serial->ucsra & (1 << UDRE0); }
inline uint8_t serial_is_transmit_complete(serial_t* serial) { return *serial->ucsra & (1 << TXC0); }

void serial_construct(serial_t* serial, uint16_t bits_per_second, uint8_t config)
{
    // Compute transmission rate
    uint16_t ubrr = F_CPU / SERIAL_MODE_ASYNCHRONOUS_DOUBLE_SCALE / bits_per_second - 1;
    serial_double_speed_enable(serial);

    // If too low or in special case for Uno firmware, no double speed
    if (ubrr > 4095 || bits_per_second == 57600)
    {
        ubrr = F_CPU / SERIAL_MODE_ASYNCHRONOUS_NORMAL_SCALE / bits_per_second - 1;
        serial_double_speed_disable(serial);
    }

    // Set transmission rate
    serial_set_ubrr(serial, ubrr);

    // Set config
    *serial->ucsrb = config;

    // Set data, parity, and stop bits
    serial_transmit_enable(serial);
    serial_receive_enable(serial);
    // serial_interrupt_receive_enable(serial);
    serial_interrupt_empty_disable(serial);
}

void serial_destroy(serial_t* serial) {
    // Wait for transmission
    serial_flush(serial);

    // Clear flags
    serial_transmit_disable(serial);
    serial_receive_disable(serial);
    // serial_interrupt_receive_disable(serial);
    serial_interrupt_empty_disable(serial);
}

inline void serial_write_internal(serial_t* serial, uint8_t data)
{
    *serial->udr = data;
    *serial->ucsra &= (1 << U2X0) | (1 << MPCM0);  // Reset UCSRA register to any writeable bits TODO: necessary?
    *serial->ucsra |= (1 << TXC0);                 // Indicate that transmission done
}

uint8_t serial_write(serial_t* serial, uint8_t data)
{
    serial->written = true;

    // If the register and buffer are empty, just send the byte immediately
    if (serial->transmit_buffer_head == serial->transmit_buffer_tail && serial_is_empty(serial))
    {
        TRANSMIT_ATOMIC
        {
            serial_write_internal(serial, data);
        }
        return 1;
    }

    // Otherwise, manipulate the buffer
    else
    {
        transmit_buffer_index_t next = (serial->transmit_buffer_head + 1) % SERIAL_TRANSMIT_BUFFER_SIZE;
        while (next == serial->transmit_buffer_tail);  // Hang if the buffer is full, interrupt will handle
        serial->transmit_buffer[serial->transmit_buffer_head] = data; // Place in buffer

        // Increment head and re-enable interrupt, cannot be interrupted
        TRANSMIT_ATOMIC
        {
            serial->transmit_buffer_head = next;
            serial_interrupt_empty_enable(serial);
        }

        return 1;
    }
}

void serial_on_empty_interrupt(serial_t* serial)
{
    // Consume byte and advance buffer tail
    uint8_t data = serial->transmit_buffer[serial->transmit_buffer_tail];
    serial->transmit_buffer_tail = (serial->transmit_buffer_tail + 1) % SERIAL_TRANSMIT_BUFFER_SIZE;

    // Send data, no atomic since we're in interrupt
    serial_write_internal(serial, data);

    // Disable interrupts if tail reaches head
    if (serial->transmit_buffer_tail == serial->transmit_buffer_head)
    {
        serial_interrupt_empty_disable(serial);
    }
}

void serial_flush(serial_t* serial)
{
    // If not written, easy short-circuit
    if (!serial->written)
    {
        return;
    }

    // Otherwise wait for interrupt disable or done transmitting
    while (serial_interrupt_empty_enabled(serial) || serial_is_transmit_complete(serial) != 0);
}
