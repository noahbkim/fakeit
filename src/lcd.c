#include "lcd.h"
#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define LCD_DATA_DIRECTION DDRD
#define LCD_DATA_BITS ((1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4))
#define LCD_DATA_PORT PORTD

#define LCD_CONTROL_DIRECTION DDRB
#define LCD_CONTROL_BITS ((1 << PB1) | (1 << PB0))
#define LCD_CONTROL_PORT PORTB

#define LCD_MODE_FLAG (1 << PB0)
#define LCD_ENABLE_FLAG (1 << PB1)

#define NOP __asm__ __volatile__ ("nop\n\t")

/// Enable the control signal
void lcd_enable()
{
    LCD_CONTROL_PORT |= LCD_ENABLE_FLAG;
}

/// Disable the control signal
void lcd_disable()
{
    LCD_CONTROL_PORT &= ~LCD_ENABLE_FLAG;
}

/// Toggle the enable signal long enough for the LCD to read data
void lcd_enable_pulse()
{
    lcd_enable();
    NOP;
    NOP;
    lcd_disable();
}

/// Send the upper four bits to the LCD and pulse enable
void lcd_send_nibble(uint8_t byte)
{
    LCD_DATA_PORT = (byte & 0xF0) + (LCD_DATA_PORT & 0x0F);
    lcd_enable_pulse();
}

/// Set the control flag
void lcd_mode_control()
{
    LCD_CONTROL_PORT &= ~LCD_MODE_FLAG;
}

/// Set the data flag
void lcd_mode_data()
{
    LCD_CONTROL_PORT |= LCD_MODE_FLAG;
}

/// Write a byte of control to the LCD
void lcd_send_control(uint8_t control)
{
    lcd_mode_control();
    lcd_send_nibble(control);
    lcd_send_nibble(control << 4);
    _delay_ms(2);
}

/// Write a byte of data to the LCD
void lcd_send_data(uint8_t data)
{
    lcd_mode_data();
    lcd_send_nibble(data);
    lcd_send_nibble(data << 4);
    _delay_ms(2);
}

/// Clear the entire display
void lcd_clear()
{
    lcd_send_control(1 << 0);
}

/// Return home
void lcd_return()
{
    lcd_send_control(1 << 1);
}

typedef enum {lcd_shift_decrement = 0, lcd_shift_increment = 1} entry_direction;

/// Set entry mode
/// Direction controls the movement of the cursor on entry
/// Shift indicates whether the contents will move on entry
void lcd_entry_mode(entry_direction direction, bool shift)
{
    lcd_send_control((1 << 2) | (direction << 1) | (shift << 0));
}

/// Display on/off toggle
void lcd_toggle_display(bool display_on, bool cursor_on, bool character_blink)
{
    lcd_send_control((1 << 3) | (display_on << 2) | (cursor_on << 1) | (character_blink << 0));
}

typedef enum {lcd_shift_cursor = 0, lcd_shift_screen = 1} to_shift;
typedef enum {lcd_shift_left = 0, lcd_shift_right = 1} shift_direction;

/// Toggle display shift
void lcd_toggle_shift(to_shift shift, shift_direction direction)
{
    lcd_send_control((1 << 4) | (shift << 3) | (direction << 2));
}

/// Shorthand write string
void lcd_write_string(char *string)
{
    uint8_t i = 0;
    while (string[i])  // Loop until next character is NULL
    {
        lcd_send_data(string[i]);  // Send the character
        i++;
    }
}

/// Move cursor
void lcd_move_cursor(uint8_t row, uint8_t col)
{
    lcd_send_control(row == 0 ? 0x80 + col : 0xc0 + col);
}


/// Initialize the LCD
// TODO: rewrite each magic control into an actual function
void lcd_setup() {
    LCD_DATA_DIRECTION |= LCD_DATA_BITS;
    LCD_CONTROL_DIRECTION |= LCD_CONTROL_BITS;
    _delay_ms(15);
    lcd_send_nibble(0x30);
    _delay_ms(4);
    lcd_send_nibble(0x30);
    _delay_us(100);
    lcd_send_nibble(0x30);
    lcd_send_nibble(0x20);
    _delay_ms(2);
    lcd_send_control(0x28);     // Function Set: 4-bit interface, 2 lines
    lcd_send_control(0x0f);     // Display and cursor on
}
