#ifndef RTC_H
#define RTC_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 2-3-2021
* File : rtc.h
***************************************************************/

/*****************
    Includes
******************/
#include <TimeLib.h>
/*****************
    Defines
******************/

/*****************
    Structs
******************/

/*************************
    Function templates
*************************/
time_t rtc_now();
int8_t rtc_currentDay();
int8_t rtc_currentHour();
int8_t rtc_currentMinute();
int8_t rtc_day(time_t tm);
int8_t rtc_month(time_t tm);
int16_t rtc_year(time_t tm);
int8_t rtc_hour(time_t tm);
int8_t rtc_minute(time_t tm);
int8_t rtc_second(time_t tm);
void rtc_setTime(time_t tm);
void rtc_setTime(char *timestr); // Only format is "2020-10-06T15:30"

#endif /* RTC_H */

