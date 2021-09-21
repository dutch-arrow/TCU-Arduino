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

/*****************
    Private data
******************/
#ifdef TOM
char SSID[] = "ASUS-fampijl";
char PASSWORD[] = "arrowfamily2014";
#else
char SSID[] = "Familiepijl";
char PASSWORD[] = "Arrow6666!";
#endif
int8_t rc;
time_t curtime;
int8_t curday;
int8_t curhour;
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

bool next_hour() {
	if (rtc_currentHour() != curhour) {
		curhour = rtc_currentHour();
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
	gen_setTraceOn(true);
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
	lcd_init();
	int8_t delay_factor = 2;
	int8_t rc = wifi_init(SSID, PASSWORD);
	while (rc != 0) {
		/*
			WL_CONNECTED       = 0
			WL_NO_SSID_AVAIL   = 1
			WL_SCAN_COMPLETED  = 2
			WL_CONNECT_FAILED  = 4
			WL_CONNECTION_LOST = 5
			WL_DISCONNECTED    = 6
			WL_AP_LISTENING    = 7
			WL_AP_CONNECTED    = 8
			WL_AP_FAILED       = 9
		*/
		if (rc == 1 || rc == 4) {
			lcd_printf(0, "%d Foute SSID", rc);
			lcd_printf(1, "of wachtwoord");
			delay(1000 * delay_factor);
			delay_factor *= 2;
		} else if (rc == 5 || rc == 6) {
			lcd_printf(0, "%d Geen netwerk.", rc);
			lcd_printf(1, "Retry na %d s", 5 * delay_factor);
			delay(5000 * delay_factor);
			delay_factor *= 2;
		} else if (rc != 3) {
			lcd_printf(0, "status=%d", rc);
			while (true) { }
		}
		rc = wifi_init(SSID, PASSWORD);
	}
	rc = wifi_setRTC();
	while (rc == -2) { // wrong response, so try again after 2 sec.
		delay(2000);
		rc = wifi_setRTC();
	}
	if (rc < 0) {
		setTime(timeConvert());
	}
	curtime = rtc_now();
	logline("It is now: %02d/%02d/%4d %02d:%02d:%02d", rtc_day(curtime), rtc_month(curtime), rtc_year(curtime),
			rtc_hour(curtime), rtc_minute(curtime), rtc_second(curtime));
	curday = rtc_day(curtime);
	curminute = rtc_minute(curtime);
	curhour = rtc_hour(curtime);
	restserver_init();
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
	// Reset time every hour on the third minute and 30 second
	if (curminute == 3 && rtc_second(curtime) == 30) {
		if (!wifi_isConnected()) {
			logline("Wifi is not available");
			// If not, try to connect again
			int8_t delay_factor = 2;
			int8_t retry = 0;
			int rc = wifi_init(SSID, PASSWORD);
			while (rc != 0 && retry < 5) {
				delay(1000 * delay_factor);
				delay_factor *= 2;
				rc = wifi_init(SSID, PASSWORD);
			}
			if (rc == 0) {
				restserver_init();
			}
		}
		if (wifi_isConnected()) {
			logline("Wifi is available");
			int8_t retries = 0;
			rc = wifi_setRTC();
			while (rc == -2 && retries < 10) { // wrong response, so try again after 2 sec.
				delay(2000);
				rc = wifi_setRTC();
			}
			if (retries != 10) {
				curtime = rtc_now();
				curday = rtc_day(curtime);
				curminute = rtc_minute(curtime);
				curhour = rtc_hour(curtime);
			}
		}
	}
	restserver_handle_request();
    gen_checkDeviceStates(curtime);
	// Every minute
    if (next_minute()) {
	    logline("A minute has passed...");
		// Check if we are still connected to Wifi
		if (!wifi_isConnected()) {
			logline("Wifi is not available");
			// If not, try to connect again
			int8_t delay_factor = 2;
			int8_t retry = 0;
			int rc = wifi_init(SSID, PASSWORD);
			while (rc != 0 && retry < 5) {
				delay(1000 * delay_factor);
				delay_factor *= 2;
				rc = wifi_init(SSID, PASSWORD);
			}
			if (rc == 0) {
				restserver_init();
			}
		} else {
			logline("Wifi is available");
		}
		// Even if wifi is not available we can still continue
		sensors_read();
		lcd_displayLine1(sensors_getTerrariumTemp(), sensors_getRoomTemp());
		char ip[16];
		wifi_getIPaddress(ip); // Will show 0.0.0.0 when no wifi available
		lcd_displayLine2(ip, "");
		tmr_check(curtime);
		rls_checkSprayerRule(curtime);
		rls_checkTempRules(curtime);
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
