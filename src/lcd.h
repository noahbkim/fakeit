#ifndef FAKEIT_LCD_H
#define FAKEIT_LCD_H

#include <stdint.h>

void lcd_send_control(uint8_t control);
void lcd_send_data(uint8_t data);
void lcd_clear();
void lcd_write_string(char *string);
void lcd_move_cursor(uint8_t row, uint8_t col);

#endif //FAKEIT_LCD_H
