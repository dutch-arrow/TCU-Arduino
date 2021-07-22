/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 3-3-2021
* File : timers.cpp
***************************************************************/

/*****************
    Includes
******************/
#include "timers.h"
#include "eeprom.h"
#include "logger.h"
#include "terrarium.h"
#include "rtc.h"
#include <JsonParser.h>
/*****************
    Private data
******************/
int8_t NR_OF_TIMERS;
static Timer timers[20];

/**********************
    Private functions
**********************/
int8_t tmr_getNrOfTimers() {
	Device *devices = gen_getDevices();
	int8_t nr = 0;
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		nr += devices[i].nr_of_timers;
	}
	if (nr > MAX_NR_OF_TIMERS) {
		logline("FATAL: Total number of timers (%d) exceeds maximum (%d)", nr, MAX_NR_OF_TIMERS);
		nr = 0;
	}
	return nr;
}

int8_t tmr_getIndex(int8_t device, int8_t index) {
	for (int i = 0; i < NR_OF_TIMERS; i++) {
		if (timers[i].device == device && timers[i].index == index) {
			return i;
		}
	}
	logline("ERROR: Invalid index: %d", index);
	return -1;
}

int8_t tmr_setTimerValues(int8_t device, int8_t index, int16_t minutes_on, int16_t minutes_off, int16_t period, int8_t repeat) {
	int8_t ix = -1;
	for (int8_t i = 0; i < NR_OF_TIMERS; i++) {
		if (timers[i].device == device && timers[i].index == index) {
			timers[i].minutes_on = minutes_on;
			timers[i].minutes_off = minutes_off;
			timers[i].on_period = period;
			timers[i].repeat_in_days = repeat;
			ix = i;
		}
	}
	return ix;
}

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
void tmr_initEEPROM() {
	NR_OF_TIMERS = tmr_getNrOfTimers();
	Device *devices = gen_getDevices();
	int ix = 0;
	for (int8_t i = 0; i < NR_OF_DEVICES; i++) {
		for (int8_t j = 1; j <= devices[i].nr_of_timers; j++) {
			timers[ix] = {.device = i, .index = j, .minutes_on = 0, .minutes_off = 0, .on_period = 0, .repeat_in_days = 0};
			ix++;
			if (ix > NR_OF_TIMERS) {
				logline("ERROR: exceeding the NR_OF_TIMERS");
				return;
			}
		}
	}
	epr_setNrOfTimersStored(NR_OF_TIMERS);
	for	(int i = 0;	i < NR_OF_TIMERS; i++) {
		epr_saveTimerToEEPROM(i, &timers[i]);
	}
    logline("EEPROM for timers initialized.");
}

void tmr_init() {
	NR_OF_TIMERS = tmr_getNrOfTimers();
	logline("Total number of timers=%d", NR_OF_TIMERS);
	logline("Timers initialized.");
	// Get timers from EEPROM
	int8_t hr_on, min_on, hr_off, min_off;
	char tmp[200];
	logline("Registered timers");
	for (int i = 0; i < NR_OF_TIMERS; i++) {
		epr_getTimerFromEEPROM(i, &timers[i]);
		if (timers[i].device != 0 &&timers[i].repeat_in_days == 1) {
			hr_on = timers[i].minutes_on / 60;
			min_on = timers[i].minutes_on - hr_on * 60;
			hr_off = timers[i].minutes_off / 60;
			min_off = timers[i].minutes_off - hr_off * 60;
			sprintf(tmp, "  d=%d i=%d, on=%02d:%02d off=%02d:%02d p=%d", 
				timers[i].device, timers[i].index, hr_on, min_on, hr_off, min_off, timers[i].on_period);
			logline(tmp);
		}
	}
}

void tmr_dump(char *prefix) {
	int8_t hr_on, min_on, hr_off, min_off;
	logline(prefix);
	char tmp[200];
	for (int i = 0; i < NR_OF_TIMERS; i++) {
		if (timers[i].device != 0 && timers[i].repeat_in_days == 1) {
			hr_on = timers[i].minutes_on / 60;
			min_on = timers[i].minutes_on - hr_on * 60;
			hr_off = timers[i].minutes_off / 60;
			min_off = timers[i].minutes_off - hr_off * 60;
			sprintf(tmp, "  d=%d i=%d, on=%02d:%02d off=%02d:%02d p=%d", 
				timers[i].device, timers[i].index, hr_on, min_on, hr_off, min_off, timers[i].on_period);
			logline(tmp);
		}
	}
}

void tmr_check(time_t curtime) {
	if (!rls_isSprayerRuleActive()) { // If sprayer rule is active, skip 
		logline("Check timers");
	    Timer curtimer;
	    Timer acttimer;
	    int8_t shouldBeOn = 0;
	    int8_t device = 0;
	    int16_t curmins = rtc_hour(curtime) * 60 + rtc_minute(curtime);
	    Timer t;
	    for (int8_t i = 0; i <= NR_OF_TIMERS; i++) {
		    if (i < NR_OF_TIMERS) {
			    t = timers[i];
			    device = t.device;
		    } else {
			    device = 0;
		    }
		    if (i > 0) {
			    if (device != curtimer.device) {
					if (!gen_isDeviceOnManual(curtimer.device)) {
						if (shouldBeOn == 1) {
							int8_t setByRule = gen_isSetByRule(acttimer.device);
							int32_t endtime = gen_getEndTime(acttimer.device);
							if (endtime == -2 && setByRule == 1) {
								rls_switchRulesetsOff(); // timer has higher prio 
							}
							if (acttimer.on_period > 0) {
								endtime = curtime + acttimer.on_period;
								gen_showState("switch on period > 0", acttimer.device);
								gen_setDeviceState(acttimer.device, endtime, setByRule);
							} else {
								gen_showState("switch on period <=0", acttimer.device);
								gen_setDeviceState(acttimer.device, -1, setByRule);
							}
						} else { // should be off
							int8_t setByRule = gen_isSetByRule(curtimer.device);
							int32_t endtime = gen_getEndTime(curtimer.device);
							gen_showState("switch off", curtimer.device);
							if (endtime == -1 && setByRule == 1) { // timer had overruled rule
								rls_switchRulesetsOn(); // rules can be activated again 
								gen_setDeviceState(curtimer.device, -2, 1); // and device is on according to rule
							}
							if (setByRule == 0) {
								gen_setDeviceState(curtimer.device, 0, 0);
							}
						}
					}
					shouldBeOn = 0;
				}
			}
			if (i < NR_OF_TIMERS) {
				if (t.repeat_in_days > 0) {
					if ((curmins >= t.minutes_on && curmins < t.minutes_off) || (curmins == t.minutes_on && t.on_period > 0)) {
						shouldBeOn = 1;
						acttimer = t;
					}
				}
				curtimer = t;
			}
	    }
	} else {
		logline("Timers not checked because sprayer rule is active");
	}
}
/*
[
    {"device": "light1","index":1,"hour_on": 9,"minute_on": 0,"hour_off":21,"minute_off": 0,"repeat": 1, "period": 0},
    {"device": "light2","index":1,"hour_on": 9,"minute_on": 30,"hour_off":21,"minute_off": 30,"repeat": 1, "period": 0},
    {"device": "sprayer","index":1,"hour_on": 0,"minute_on": 0,"hour_off":0,"minute_off": 0,"repeat": 0, "period": 0},
    {"device": "sprayer","index":2,"hour_on": 0,"minute_on": 0,"hour_off":0,"minute_off": 0,"repeat": 0, "period": 0},
    {"device": "sprayer","index":3,"hour_on": 0,"minute_on": 0,"hour_off":0,"minute_off": 0,"repeat": 0, "period": 0}
]
*/
void tmr_setTimersFromJson(char *json) {
	JsonParser<100> parser;
	JsonArray timerArray = parser.parseArray(json);
	if (!timerArray.success()) {
		// create the error response
		sprintf(json, "{\"error_msg\":\"Could not deserialize the JSON\"}");
		logline("deserializeJson() failed");
		return;
	} else {
		char tmp[100];
		for (int8_t i = 0; i < timerArray.getLength(); i++) {
			JsonHashTable tmr = timerArray.getHashTable(i);
			int8_t dev = gen_getDeviceIndex(tmr.getString("device"));
			int8_t ix = tmr.getLong("index");
			int8_t hr_on = tmr.getLong("hour_on");
			int8_t min_on = tmr.getLong("minute_on");
			int8_t hr_off = tmr.getLong("hour_off");
			int8_t min_off = tmr.getLong("minute_off");
			int8_t repeat = tmr.getLong("repeat");
			int16_t period = tmr.getLong("period");
			int16_t all_on = hr_on * 60 + min_on;
			int16_t all_off = hr_off * 60 + min_off;
			int8_t tix = tmr_setTimerValues(dev, ix, all_on, all_off, period, repeat);
			// tmp[0] = 0;
			// tmr_getTimerAsJson(dev, ix, tmp);
			// Serial1.println(tmp);
			epr_saveTimerToEEPROM(tix, &timers[tix]);
			// Timer t;
			// epr_getTimerFromEEPROM(tix, &t);
			// tmp[0] = 0;
			// tmr_getTimerAsJson(&t, tmp);
			// Serial1.println(tmp);
		}
		sprintf(json, "");
	}
}

void tmr_getTimerAsJson(Timer *t, char *json) {
	Device *devices = gen_getDevices();
	int8_t hr_on, min_on, hr_off, min_off;
	hr_on = t->minutes_on / 60;
	min_on = t->minutes_on - hr_on * 60;
	hr_off = t->minutes_off / 60;
	min_off = t->minutes_off - hr_off * 60;
	char tmp[200];
	sprintf(tmp,
		"{\"device\":\"%s\",\"index\":%d,\"hour_on\":%d,\"minute_on\":%d,\"hour_off\":%d,\"minute_off\":%d,\"repeat\":%d,\"period\":%d}",
		devices[t->device].name, t->index, hr_on, min_on, hr_off, min_off, t->repeat_in_days, t->on_period);
	strcat(json, tmp);
}

void tmr_getTimerAsJson(int8_t dev, int8_t ix, char *json) {
	Device *devices = gen_getDevices();
	if (devices[dev].nr_of_timers > 0 && ix > 0 && devices[dev].nr_of_timers <= ix) {
		int8_t tix = tmr_getIndex(dev, ix);
		if (tix != -1) {
			Timer t = timers[tix];
			tmr_getTimerAsJson(&t, json);
		} else {
			logline("ERROR: no timer for dev=%d, index=%d", dev, ix);
		}
	}
}

void tmr_getTimersAsJson(char *device, char *json) {
	Device *devices = gen_getDevices();
	int8_t dev = gen_getDeviceIndex(device);
	strcpy(json, "[");
	if (devices[dev].nr_of_timers > 0) {
		int8_t rc = 0;
		int8_t n = devices[dev].nr_of_timers;
		for (int i = 0; i < n; i++) {
			int8_t ix = tmr_getIndex(dev, i + 1);
			if (ix != -1) {
				tmr_getTimerAsJson(&timers[ix], json);
				if (i != (n - 1)) {
					strcat(json, ",");
				}
			}
		}
	}
	strcat(json, "]");
}
