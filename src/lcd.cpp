/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : TP
* Created On : 18-2-2021
* File : lcd.cpp
***************************************************************/

/*****************
    Includes
******************/
#ifndef SIMULATION
#include <Arduino.h>
#include <LCD_I2C.h>
#endif
#include "logger.h"
#include "rtc.h"
#include "lcd.h"
#include "terrarium.h"

/*****************
    Private data
******************/
LCD_I2C lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display
// Special characters
uint8_t degrees[8] = {0x6, 0x9, 0x9, 0x6, 0x0, 0x0, 0x0}; 
int8_t ix;
char textline2[200];
/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/

void lcd_init() {
    lcd.begin(); // initialize the lcd
    lcd.createChar(0, degrees);
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    ix = 0;
    logline("LCD initialized.");
}

void lcd_clearLine(int8_t nr) {
    lcd.setCursor(0, nr);
    lcd.print("                ");
    lcd.setCursor(0, nr);
}

void lcd_printf(int8_t nr, char *format, ...) {
    char tmp[250];
    va_list l_Arg;
    va_start(l_Arg, format);
    vsprintf(tmp, format, l_Arg);
    lcd_clearLine(nr);
    lcd.print(tmp);
}

void lcd_displayLine1(int8_t t_terrarium, int8_t t_room) {
    // display terrarium sensor readings always on line 1 of LCD
    lcd_clearLine(0);
    char tmp[250];
    sprintf(tmp, "Kmr:%2d", t_room);
    lcd.print(tmp); // 6 chars
    lcd.write(0);
    lcd.print("C"); // 2 characters
    sprintf(tmp, " Ter:%2d", t_terrarium);
    lcd.print(tmp); // 6 chars
    lcd.write(0);
    lcd.print("C"); // 2 characters
    // Total: 16 chars
}

void lcd_displayLine2(char *ipaddr, char *txt) {
	// display datetime
	lcd_clearLine(1);
	time_t curtime = rtc_now();
	sprintf(textline2, "%02d/%02d/%4d %02d:%02d ", rtc_day(curtime), rtc_month(curtime), rtc_year(curtime), rtc_hour(curtime), rtc_minute(curtime));
	strcat(textline2, ipaddr);
	strcat(textline2, " ");
	strcat(textline2, txt);
	ix = 0;
}

void lcd_rotate() {
    int len = strlen(textline2);
    char ln[17];
    for (int i = 0; i < 16; i++) {
        ln[i] = textline2[(ix + i) % len];
    }
    ln[16] = 0;
    lcd_clearLine(1);
    lcd.print(ln);
    ix = (ix + 1) % len;
}