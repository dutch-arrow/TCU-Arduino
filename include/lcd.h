#ifndef LCD_H
#define LCD_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : TP
* Created On : 18-2-2021
* File : lcd.h
***************************************************************/

/*****************
    Includes
******************/

/*****************
    Defines
******************/

/*****************
    Structs
******************/

/*************************
    Function templates
*************************/
/*
* Initialize the LCD
*/
void lcd_init();
/*
* Clear a line on the display.
* 
* param(in) nr line number (0 is first line)
*/
void lcd_clearLine(int8_t nr);
/*
* Formatted print on given line.
*
* param(in) nr      line number (0 is first line)
* param(in) format  format string
* param(in) ...     arguments used in format string
*/
void lcd_printf(int8_t nr, char *format, ...);
/*
* Display first line of display
*
* param(in) t_terrarium1  terrarium1 temperature
* param(in) t_terrarium2  terrarium2 temperature
* param(in) t_room        room temperature
*/
void lcd_displayLine1(int8_t t_terrarium, int8_t t_room);
/*
* Display second line of display
*
* param(in) ipaddr       IP address
* param(in) txt          any other text
*/
void lcd_displayLine2(char *ipaddr, char *txt);
/*
* Shift text line 2 one character to the left
*/
void lcd_rotate();

#endif /* LCD_H */
