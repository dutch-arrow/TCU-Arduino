/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 25-2-2021
* File : sensors.cpp
***************************************************************/

/*****************
    Includes
******************/
#ifndef SIMULATION
#include <DHT.h>
#include <DallasTemperature.h>
#include <TimeLib.h>
#endif
#include "logger.h"
#include "terrarium.h"
#include "sensors.h"
/*****************
    Private data
******************/
DHT dht(pin_sensor_out);
OneWire ow(pin_sensor_in);
DallasTemperature ds(&ow);
bool room_sensor;
int8_t room_temp;
int8_t room_hum;
bool terrarium_sensor;
int8_t terrarium_temp;

/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
void sensors_init() {
	dht.begin(65);
	room_sensor = true;
	ds.begin();
	terrarium_sensor = true;
	logline("Sensors initialized.");
}

void sensors_read() {
	float hr = 0.0;
	float tr = 0.0;
	float tt = 0.0;
	if (room_sensor) {
		// Room sensor
		hr = dht.readHumidity();
		tr = dht.readTemperature();
		room_temp = (isnan(tr) ? 0 : round(tr));
		room_hum = (isnan(hr) ? 0 : round(hr));
	}
	if (terrarium_sensor) {
		// Terrarium1 sensor
		ds.requestTemperatures();
		tt = ds.getTempCByIndex(0); 
		terrarium_temp = (int8_t)round(tt);
		if (terrarium_temp < 0) {
			terrarium_temp = 0;
		}
	}
	logline("Temp Terrarium=%.1f, Room=%.1f , Hum Room=%.1f", tt, tr, hr);
}

void sensors_tojson(char *json) {
    char tmp[100];
	int32_t curtime = now();
	// DD-MMM-YYYY hh:mm
    sprintf(tmp, "{\"clock\":\"%02d-%02d-%4d %02d:%02d\",\"sensors\":", day(curtime), month(curtime), year(curtime), hour(curtime), minute(curtime));
    strcpy(json, tmp);
    sprintf(tmp,"[{\"location\":\"room\",\"temperature\":%d,\"humidity\":%d},", room_temp, room_hum);
    strcat(json, tmp);
    sprintf(tmp,"{\"location\":\"terrarium\",\"temperature\":%d}]}", terrarium_temp);
    strcat(json, tmp);
}

void sensors_setTestValues(char *testurl) {
	// testurl: [room]/[tt]
	room_temp = atoi(strtok(testurl, "/"));
	room_sensor = false;
	terrarium_temp = atoi(strtok(NULL, "/"));
	terrarium_sensor = false;
	logline("Room temp is now %d, terrarium temp is now %d", room_temp, terrarium_temp);
}

void sensors_setTestOff() {
	room_sensor = true;
	terrarium_sensor = true;
}

int8_t sensors_getRoomTemp() {
	return room_temp;
}

int8_t sensors_getTerrariumTemp() {
	return terrarium_temp;
}

