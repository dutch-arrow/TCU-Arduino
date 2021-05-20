/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 2-3-2021
* File : rtc.cpp
***************************************************************/

/*****************
    Includes
******************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rtc.h"
/*****************
    Private data
******************/

/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
time_t rtc_now() {
	return now();
}
int8_t rtc_currentDay() {
	return day(now());
}
int8_t rtc_currentHour() {
	return hour(now());
}
int8_t rtc_currentMinute() {
	return minute(now());
}
int8_t rtc_day(time_t tm) {
	return day(tm);
}
int8_t rtc_month(time_t tm) {
	return month(tm);
}
int16_t rtc_year(time_t tm) {
	return year(tm);
}
int8_t rtc_hour(time_t tm) {
	return hour(tm);
}
int8_t rtc_minute(time_t tm) {
	return minute();
}
int8_t rtc_second(time_t tm) {
	return second(tm);
}
void rtc_setTime(time_t tm) {
    setTime(tm);
}
void rtc_setTime(char *dt) {
	char tmp[5];
	tmElements_t tmel;
	strncpy(tmp, dt, 4);
	tmp[4] = 0;
	tmel.Year = atoi(tmp) - 1970;
	strncpy(tmp, dt + 5, 2);
	tmp[2] = 0;
	tmel.Month = atoi(tmp);
	strncpy(tmp, dt + 8, 2);
	tmp[2] = 0;
	tmel.Day = atoi(tmp);
	strncpy(tmp, dt + 11, 2);
	tmp[2] = 0;
	tmel.Hour = atoi(tmp);
	strncpy(tmp, dt + 14, 2);
	tmp[2] = 0;
	tmel.Minute = atoi(tmp);
   
	int32_t tm = makeTime(tmel);
	setTime(tm);
}

