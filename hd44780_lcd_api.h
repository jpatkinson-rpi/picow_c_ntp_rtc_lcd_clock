/*******************************************************************
*
* hd44780_lcd_api.h
*
* HD44780 LCD controller API commands
*
********************************************************************/
#ifndef __HD44780_LCD_API_H__

#define __HD44780_LCD_API_H__

#define HD44780_MAX_LINES      2
#define HD44780_MAX_CHARS      16

void i2c_write_byte( uint8_t val );
void hd44780_lcd_toggle_enable( uint8_t val );
void hd44780_lcd_send_byte( uint8_t val, int mode ); 
void hd44780_lcd_clear( void ); 
void hd44780_lcd_set_cursor( int row, int column );
void hd44780_lcd_char( char val ); 
void hd44780_lcd_string( const char *s ); 
void hd44780_lcd_init();

#endif // __HD44780_LCD_API_H__
