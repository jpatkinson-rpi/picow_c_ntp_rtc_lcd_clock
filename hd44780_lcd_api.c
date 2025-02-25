/*******************************************************************
*
* hd44780_lcd_api.c
*
********************************************************************/
#include <stdio.h>
#include <stdint.h>

#include "hardware/i2c.h"

#include "hd44780_lcd_defs.h"
#include "hd44780_lcd_api.h"

/* 
   Implements a HD44780 character LCD connected via PCF8574 on I2C
      - GPIO 4 (pin 6) -> LCD I2C SDA
      - GPIO 5 (pin 7) -> LCD I2C SCL 
      - 3.3V (pin 36)  -> LCD VCC
      - 0V (pin 38)    -> LCD GND
*/

void i2c_write_byte( uint8_t val ) 
{
    i2c_write_blocking( PICO_DEFAULT_I2C_INSTANCE(), HD44780_LCD_I2C_ADDR, &val, 1, false );
}

void hd44780_lcd_toggle_enable( uint8_t val ) 
{
    // Toggle enable pin on LCD display
    sleep_us( HD44780_ENABLE_DELAY_US );
    i2c_write_byte( val | HD44780_LCD_ENABLE_BIT );
    sleep_us( HD44780_ENABLE_DELAY_US );
    i2c_write_byte( val & ~HD44780_LCD_ENABLE_BIT );
    sleep_us( HD44780_ENABLE_DELAY_US );
}

// The display is sent a byte as two separate nibble transfers
void hd44780_lcd_send_byte( uint8_t val, int mode ) 
{
    uint8_t upper_nibble;
    uint8_t lower_nibble;

    upper_nibble = mode | (val & 0xF0) | HD44780_LCD_BACKLIGHT;
    lower_nibble = mode | ((val << 4) & 0xF0) | HD44780_LCD_BACKLIGHT;
    
    i2c_write_byte( upper_nibble );
    hd44780_lcd_toggle_enable( upper_nibble );
    i2c_write_byte( lower_nibble );
    hd44780_lcd_toggle_enable( lower_nibble );
}

void hd44780_lcd_clear( void)  
{
    hd44780_lcd_send_byte( HD44780_LCD_CLEAR_DISPLAY, HD44780_LCD_COMMAND );
}

// set LCD cursor position
void hd44780_lcd_set_cursor( int row, int column ) 
{
    int cmd;
    if ( row == 0 )
        cmd = 0x80 + column;
    else
        cmd = 0xC0 + column;
    hd44780_lcd_send_byte( cmd, HD44780_LCD_COMMAND );
}

void hd44780_lcd_char( char val ) 
{
    hd44780_lcd_send_byte( val, HD44780_LCD_CHARACTER );
}

void hd44780_lcd_string( const char *s ) 
{
    while ( *s ) 
    {
        hd44780_lcd_char( *s++ );
    }
}

void hd44780_lcd_init() 
{
    /* 4-bit reset sequence */
    hd44780_lcd_send_byte(0x03, HD44780_LCD_COMMAND);
    hd44780_lcd_send_byte(0x03, HD44780_LCD_COMMAND);
    hd44780_lcd_send_byte(0x03, HD44780_LCD_COMMAND);
    hd44780_lcd_send_byte(0x02, HD44780_LCD_COMMAND);

    /* initialise LCD display */ 
    hd44780_lcd_send_byte(HD44780_LCD_ENTRY_MODE_SET | HD44780_LCD_ENTRY_LEFT, HD44780_LCD_COMMAND);
    hd44780_lcd_send_byte(HD44780_LCD_FUNCTION_SET | HD44780_LCD_FUNCTION_2LINE, HD44780_LCD_COMMAND);
    hd44780_lcd_send_byte(HD44780_LCD_ON_DISPLAY_CONTROL | HD44780_LCD_ON_DISPLAY, HD44780_LCD_COMMAND);
    
    /* clear LCD */
    hd44780_lcd_clear();
}

