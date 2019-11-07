#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_NONE 255
#define LCD_SELECT 206
#define LCD_LEFT 156
#define LCD_UP 51
#define LCD_DOWN 106
#define LCD_RIGHT 0

void lcd_send_control(uint8_t control);
void lcd_send_data(uint8_t data);
void lcd_clear();
void lcd_write_string(char *string);
void lcd_move_cursor(uint8_t row, uint8_t col);

uint8_t lcd_button(uint8_t adc_value);

#endif  // LCD_H
