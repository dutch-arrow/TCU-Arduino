#ifndef TIMERS_H
#define TIMERS_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 3-3-2021
* File : timers.h
***************************************************************/

/*****************
    Includes
******************/
#include <stdint.h>
#include "rtc.h"

/*****************
    Defines
******************/

/*****************
    Structs
******************/
typedef struct {
    int8_t device;
    int8_t index;
    int16_t minutes_on;    // max 1440 = hours * 60 + minutes
    int16_t minutes_off;   // max 1440 = hours * 60 + minutes
    int16_t on_period;     // max 3600 sec = 1 hour
    int8_t repeat_in_days; // 1 - 7, default 1
} Timer;

/*************************
    Function templates
*************************/
void tmr_initEEPROM();
void tmr_init();
void tmr_setTimersFromJson(char *json);
void tmr_getTimerAsJson(int8_t device, int8_t ix, char *json);
void tmr_getTimerAsJson(Timer *t, char *json);
void tmr_getTimersAsJson(char *device, char *json);
void tmr_check(time_t curtime);
void tmr_dump(char *prefix);

#endif /* TIMERS_H */
