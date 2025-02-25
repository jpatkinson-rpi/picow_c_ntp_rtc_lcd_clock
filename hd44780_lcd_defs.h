/*******************************************************************
*
* hd44780_lcd_defs.h
*
* HD44780 LCD controller command set defines
*
********************************************************************/
#ifndef __HD44780_LCD_DEFS_H__
#define __HD44780_LCD_DEFS_H__

// HD44780 LCD controller command set defines

// clear display
#define HD44780_LCD_CLEAR_DISPLAY   0x01

//return to home position
#define HD44780_LCD_RETURN_HOME     0x02

// display entry mode bits
#define HD44780_LCD_ENTRY_MODE_SET 0x04 // DB2: set entry mode
#define HD44780_LCD_ENTRY_LEFT     0x02 // DB1: increment
#define HD44780_LCD_ENTRY_SHIFT    0x01 // DB0: shift

// display and cursor control bits
#define HD44780_LCD_ON_DISPLAY_CONTROL 0x08
#define HD44780_LCD_ON_DISPLAY         0x04  // DB2: turn display on
#define HD44780_LCD_ON_CURSOR          0x02  // DB1: turn cursor on
#define HD44780_LCD_ON_BLINK           0x01  // DB0: blinking cursor

// display and cursor shift bits
#define HD44780_LCD_MOVE_CURSOR  0x10 // DB4: move cursor/display
#define HD44780_LCD_MOVE_DISPLAY 0x08 // DB3: move display (0-> move cursor)
#define HD44780_LCD_MOVE_RIGHT   0x04 // DB2: move right (0-> left)

// function set bits
#define HD44780_LCD_FUNCTION_SET       0x20 // DB5: function set
#define HD44780_LCD_FUNCTION_8BIT_MODE 0x10 // DB4: set 8BIT mode (0->4BIT mode)
#define HD44780_LCD_FUNCTION_2LINE     0x08 // DB3: two lines (0->one line)
#define HD44780_LCD_FUNCTION_5x10      0x04 // DB2: 5x10 font (0->5x7 font)

// set CG RAM address
#define HD44780_LCD_SET_CGRAM_ADDR  0x40

// set DD RAM address
#define HD44780_LCD_SET_DDRAM_ADDR  0x80 

// backlight control
#define HD44780_LCD_BACKLIGHT 0x08

#define HD44780_LCD_ENABLE_BIT 0x04

#define HD44780_LCD_I2C_ADDR 0x27 // Default I2C address

// Modes for lcd_send_byte
#define HD44780_LCD_CHARACTER  1
#define HD44780_LCD_COMMAND    0

#define HD44780_ENABLE_DELAY_US 500  // Minimum ENABLE cycle

#endif // __HD44780_LCD_DEFS_H__
