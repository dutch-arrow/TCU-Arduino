/**************************************************************
 *
 * Copyright © 2021 Dutch Arrow Software - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the Apache Software License 2.0.
 *
 * Author : TP
 * Created On : 17/02/2021
 * File : wifi.cpp
 ***************************************************************/
/*****************
    Includes
******************/
#include <stdint.h>

#include "config.h"
#include "lcd.h"
#include "logger.h"
#include "restserver.h"
#include "rtc.h"
#include "sensors.h"
#include "terrarium.h"
#include "wifi.h"
#include "eeprom.h"
#include "timers.h"
#include "rules.h"

//#define INIT_EEPROM = true

/*****************
    Private data
******************/
int8_t rc;
time_t curtime;
int8_t curday;
int8_t curminute;

/**********************
    Private functions
**********************/
time_t timeConvert() {
	char s_month[5];
	int month, day, year, hr, min, sec;
	tmElements_t tmel;
	static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	sscanf(__DATE__, "%s %d %d", s_month, &day, &year);
	sscanf(__TIME__, "%d:%d:%d", &hr, &min, &sec);
	month = (strstr(month_names, s_month) - month_names) / 3 + 1;
	tmel.Hour = hr;
	tmel.Minute = min;
	tmel.Second = sec;
	tmel.Month = month;
	tmel.Day = day;
	// year can be given as full four digit year or two digts (2010 or 10
	// for 2010);
	// it is converted to years since 1970
	if (year > 99)
		tmel.Year = year - 1970;
	else
		tmel.Year = year + 30;

	return makeTime(tmel);
}

bool next_minute() {
	if (rtc_currentMinute() != curminute) {
		curminute = rtc_currentMinute();
		return true;
	} else {
		return false;
	}
}

bool new_day() {
	if (rtc_currentDay() != curday) {
		curday = rtc_currentDay();
		return true;
	} else {
		return false;
	}
}

/**********************
    Public functions
**********************/
void setup() {
	gen_setTraceOn(false);
	// Initialize GPIO
	gen_setup();
	// Initialize Serial1
	while (Serial1.read() == 0) {
		delay(100);
	}
	rc = 0;
	// Initialize the serial output
	Serial1.begin(9600);
	while (Serial1.read() == 0) {
		delay(100);
	}
	Serial1.println();
	logline("Trace on? %s", gen_isTraceOn() ? "yes" : "no");
	// Initialize the WIFI
	rc = wifi_init();
	if (rc != 0) {
		if (rc == 1) {
			logline("ERROR: Wifi module is broken.");
		} else {
			logline("ERROR: Cannot connect to local network after 10 tries.");
		}
		return;
	}
	// Set date and time
	int8_t retries = 0;
	rc = wifi_setRTC();
	while (rc != 0 && retries < 5) {
		if (rc == -1) {
			logline("Could not set time.");
			// Could not set the time, so use the flash time
			setTime(timeConvert());
		} else if (rc == -2) {
			// Received an error response so try again
			retries++;
			delay(1000);
			rc = wifi_setRTC();
		}
	}
	if (retries == 5) {
		setTime(timeConvert());
	}
	curtime = rtc_now();
	logline("It is now: %02d/%02d/%4d %02d:%02d:%02d", rtc_day(curtime), rtc_month(curtime), rtc_year(curtime),
			rtc_hour(curtime), rtc_minute(curtime), rtc_second(curtime));
	curday = rtc_day(curtime);
	curminute = rtc_minute(curtime);
	restserver_init();
	lcd_init();
	sensors_init();
	epr_init();
#ifdef INIT_EEPROM
	// Initialize EEPROM
	gen_initEEPROM();
	tmr_initEEPROM();
	rls_initEEPROM();
#endif
	gen_init();
	tmr_init();
	rls_init();

	sensors_read();
	lcd_displayLine1(sensors_getTerrariumTemp(), sensors_getRoomTemp());
	char ip[16];
	wifi_getIPaddress(ip);
	lcd_displayLine2(ip, "");
}

void loop() {
	// Every second
	curtime = rtc_now();
	restserver_handle_request();
    gen_checkDeviceStates(curtime);
	// Every day
	if (new_day()) {
	    logline("A day has passed...");
		if (wifi_isConnected()) {
			wifi_setRTC();  // reset the time
		}
	}
	// Every minute
    if (next_minute()) {
	    logline("A minute has passed...");
		sensors_read();
		lcd_displayLine1(sensors_getTerrariumTemp(), sensors_getRoomTemp());
		char ip[16];
		wifi_getIPaddress(ip);
		lcd_displayLine2(ip, "");
		rls_checkTempRules(curtime);
		tmr_check(curtime);
		gen_increase_time_on();
    }
    int dly = 500; // must be < 1000 = 1 second
    int8_t steps = 1000 / dly;
    for (int8_t i = 0; i < steps; i++) {
	    lcd_rotate();
	    delay(dly);
    }
	// Every second
}
