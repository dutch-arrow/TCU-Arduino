/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 26-2-2021
* File : terrarium.cpp
***************************************************************/

/*****************
    Includes
******************/
#include "Arduino.h"
#include "terrarium.h"
#include "logger.h"
#include "eeprom.h"

/*****************
    Private data
******************/
// EEPROM 250 - 80 = 170 bytes => max 170 / 9 = 18 timers 
Device devices[] = {
    {"light1",  pin_light1,  1, 0, 0, 0, 0, false},
    {"light2",  pin_light2,  1, 0, 0, 0, 0, false},
    {"light3",  pin_light3,  1, 0, 0, 0, 0, false},
    {"light4",  pin_light4,  1, 0, 0, 0, 0, false},
    {"light5",  pin_light5,  1, 0, 0, 1, 0, false},
    {"light6",  pin_light6,  0, 0, 0, 0, 0, false},
    {"fan_in",  pin_fan_in,  3, 0, 0, 0, 0, false},
    {"fan_out", pin_fan_out, 3, 0, 0, 0, 0, false},
    {"sprayer", pin_sprayer, 1, 0, 0, 0, 0, false},
    {"mist",    pin_mist,    3, 0, 0, 0, 0, false},
    {"pump",    pin_pump,    3, 0, 0, 0, 0, false}};
bool traceon = true;
extern int8_t NR_OF_TIMERS;

/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
void gen_initEEPROM() {
	// Clear the lifecycle counter
	epr_clearHoursOn();
}
void gen_init() {
    for (int i = 0; i < NR_OF_DEVICES; i++) {
	    if (devices[i].lcc) {
		    devices[i].on_time = 0;
			break;
		}
    }
}
void gen_setup() {
    pinMode(pin_light1, OUTPUT);
 //   digitalWrite(pin_light1, HIGH); // Tim
    digitalWrite(pin_light1, LOW);  //Bjorn
    pinMode(pin_light2, OUTPUT);
 //   digitalWrite(pin_light2, HIGH); // Tim
    digitalWrite(pin_light2, LOW);
    pinMode(pin_light3, OUTPUT);
//    digitalWrite(pin_light3, HIGH); // Tim
    digitalWrite(pin_light3, HIGH);   // Tim
    pinMode(pin_light4, OUTPUT);
    digitalWrite(pin_light4, HIGH);   // Tim
    pinMode(pin_light5, OUTPUT);
//    digitalWrite(pin_light5, HIGH);  // Tim
    digitalWrite(pin_light5, LOW);
    pinMode(pin_light6, OUTPUT);
    digitalWrite(pin_light6, HIGH);   // Tim
	pinMode(pin_sprayer, OUTPUT);
    digitalWrite(pin_sprayer, LOW);
	pinMode(pin_mist, OUTPUT);
    digitalWrite(pin_mist, LOW);
	pinMode(pin_pump, OUTPUT);
    digitalWrite(pin_pump, LOW);
	pinMode(pin_fan_in, OUTPUT);
    digitalWrite(pin_fan_in, LOW);
	pinMode(pin_fan_out, OUTPUT);
    digitalWrite(pin_fan_out, LOW);
}

Device *gen_getDevices() {
	return devices;
}

int8_t gen_getDeviceIndex(char *device) {
	if (strcmp(device, "no device") == 0) {
		return -1;
	} else {
		for (int i = 0; i < NR_OF_DEVICES; i++) {
			if (strcmp(devices[i].name, device) == 0) {
				return i;
			}
		}
	}
}

bool gen_isTraceOn() {
	return traceon;
}

void gen_setTraceOn(bool on) {
	traceon = on;
}

void gen_getProperties(char *json) {
	char temp[100];
	strcpy(json, "{\"tcu\":\"TERRARIUM\",");
	sprintf(temp, "\"eeprom_write_count\":%lu,\"nr_of_timers\":%d,",  epr_getEEPROMWriteCounter(), NR_OF_TIMERS);
	strcat(json, temp);
	sprintf(temp, "\"nr_of_programs\":%d,", NR_OF_RULESETS);
	strcat(json, temp);
	strcat(json, "\"devices\": [");
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		sprintf(temp, "{\"device\":\"%s\", \"nr_of_timers\":%d, \"lc_counted\": \"%s\"}", 
			devices[i].name, devices[i].nr_of_timers, devices[i].lcc ? "true" : "false");
		strcat(json, temp);
		if (i != NR_OF_DEVICES - 1) {
			strcat(json, ",");
		}
	}
	strcat(json, "]}");
}

void gen_getDeviceStates(char *json) {
	char temp[100];
	strcpy(json, "[");
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		Device dev = devices[i];
		int32_t cntr;
		if (dev.lcc) {
			cntr = epr_getHoursOn();
		} else {
			cntr = 0;
		}
		int16_t onTime = dev.on_time / 60;
		char man[20];
		sprintf(man,"\"manual\":\"%s\"}", dev.manual ? "yes" : "no");
		if (dev.end_time > 0) {		  // an endtime is defined
			time_t tm = dev.end_time; // seconds since 1-1-1970
			sprintf(temp, "{\"device\":\"%s\",\"state\":\"on\",\"end_time\":\"%02d:%02d:%02d\",\"hours_on\":%d,",
				dev.name, hour(tm), minute(tm), second(tm), cntr);
		} else if (dev.end_time == 0) { // off
			sprintf(temp, "{\"device\":\"%s\",\"state\":\"off\",\"hours_on\":%d,",
				dev.name, cntr);
		} else if (dev.end_time == -1) { // on, but endless
			sprintf(temp, "{\"device\":\"%s\",\"state\":\"on\",\"end_time\":\"no endtime\",\"hours_on\":%d,",
				dev.name, cntr);
		} else if (dev.end_time == -2) { // on, untill ideal value is reached
			sprintf(temp, "{\"device\":\"%s\",\"state\":\"on\",\"end_time\":\"until ideal temperature is reached\",\"hours_on\":%d,",
				dev.name, cntr);
		}
		strcat(temp, man);
		strcat(json, temp);
		if (i != NR_OF_DEVICES - 1) {
			strcat(json, ",");
		}
	}
	strcat(json, "]");
}

bool gen_isDeviceOn(int8_t device) {
	return devices[device].end_time != 0;
}

int32_t gen_getEndTime(int8_t device) {
	return devices[device].end_time;
}

int8_t gen_isSetByRule(int8_t device) {
	return devices[device].temprule == 0 ? 0 : 1;
}

void gen_setDeviceToManual(int8_t device, bool yes) {
	devices[device].manual = yes;
}
bool gen_isDeviceOnManual(int8_t device) {
	return devices[device].manual;
}

// end_time = 0 -> off, = -1 -> on, endless, = -2 -> on, until ideal value, >0 -> on, seconds from 1-1-1970
void gen_setDeviceState(int8_t device, int32_t end_time, int8_t temprule) {
	if (device != -1) {
		// Check if device state needs to be changed
		if (devices[device].end_time != end_time) {
			if (device < 6) {
//				digitalWrite(devices[device].pin_nr, (end_time == 0 ? HIGH : LOW)); //Tim
				digitalWrite(devices[device].pin_nr, (end_time == 0 ? LOW : HIGH)); //Bjorn
			} else {
				digitalWrite(devices[device].pin_nr, (end_time == 0 ? LOW : HIGH));
			}
			devices[device].end_time = end_time;
			if (end_time == 0) {
				devices[device].temprule = 0;
			} else {
				devices[device].temprule = temprule;
			}
			char tm[15];
			if (end_time > 0) {
				sprintf(tm, "until %02d:%02d:%02d", hour(end_time), minute(end_time), second(end_time));
			}
			logline("* Device '%s' is switched %s %s", devices[device].name,
				(end_time == 0 ? "off" : "on"),
				(end_time == 0 ? "" : (end_time == -1 ? "permanently" : (end_time == -2 ? "until ideal value is reached" : tm))));
			// Special actions
			if (device == gen_getDeviceIndex("sprayer") && end_time == 0) { // sprayer is switched off
				rls_startSprayerRule(rtc_now());
			}
			if (device == gen_getDeviceIndex("mist") && end_time != 0) { // mist is switched on
				rls_switchRulesetsOff();
			} else if (device == gen_getDeviceIndex("mist") && end_time == 0) { // mist is switched off
				rls_switchRulesetsOn();
			}
		}
	}
}

void gen_setDeviceState(char *devurl) {
	// 'device/on' or 'device/on/xxx' or 'device/off' or 'device/manual' or 'device/auto'
	char *device = strtok(devurl, "/");
	int8_t dev = -1;
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		if (strcmp(devices[i].name, device) == 0) {
			dev = i;
			break;
		}
	}
	if (dev != -1) {
		int32_t endTime = 0;
		char *action = strtok(NULL, "/");
		if (strcmp(action, "on") == 0) {
			endTime = -1;
			char *period = strtok(NULL, "/");
			if (period != NULL) {
				endTime = now() + atoi(period);
			}
			gen_setDeviceState(dev, endTime, false);
		} else if (strcmp(action, "off") == 0) {
			gen_setDeviceState(dev, 0, false);
		} else if (strcmp(action, "manual") == 0) {
			gen_setDeviceToManual(dev, true);
		} else if (strcmp(action, "auto") == 0) {
			gen_setDeviceToManual(dev, false);
		}
	}
}

void gen_checkDeviceStates(time_t curtime) {
	// Checked every second!
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		if (devices[i].end_time > 0 && curtime > devices[i].end_time) {
			gen_setDeviceState(i, 0, 0); // switch device off
		}
	}
}

void gen_increase_time_on() {
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		if (devices[i].lcc && devices[i].end_time != 0) {
			devices[i].on_time += 1; // executed every minute
			if (devices[i].on_time > 120) {
				logline("Decrease lifetime with 2 hours");
				epr_decreaseHoursOn(2); // save every 2 hours
				devices[i].on_time = 0;
			}
		}
	}
}

void gen_setCounter(char *devurl) {
	char *device = strtok(devurl, "/");
	// Get only the lifecycle devices
	char lccdev[10];
	int8_t lcc = 0;
	for (int i = 0; i < NR_OF_DEVICES; i++) {
		if (devices[i].lcc) {
			strcpy(lccdev, devices[i].name);
			break;
		}
	}
	logline("lcc device is '%s'", lccdev);
	if (strcmp(lccdev, device) == 0) {
		int32_t value = atoi(strtok(NULL, "/"));
		epr_setHoursOn(value);
		logline("Counter for device '%s' is set to %d", lccdev, value);
	}
}

void gen_showState(char * txt, int8_t device) {
	char state1[35];
	if (devices[device].end_time > 0) {
		tmElements_t tm;
		breakTime(devices[device].end_time, tm);
		sprintf(state1, "on until %02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
	} else if (devices[device].end_time == -1) {
		strcpy(state1, "always on");
	} else if (devices[device].end_time == -2) {
		strcpy(state1, "on until ideal value is reached");
	}
	char state2[25];
	if (devices[device].temprule == 0) {
		strcpy(state2, "a timer");
	} else if (devices[device].temprule != 0) {
		strcpy(state2, "a rule");
	}
	if (devices[device].end_time == 0) {
		logline("  ->%s : Device %s (%s) is off", txt, devices[device].name, devices[device].manual ? "manual" : "auto");
	} else {
		logline("  ->%s : Device %s (%s) is %s set by %s", txt, devices[device].name, devices[device].manual ? "manual" : "auto", state1, state2);
	}
}