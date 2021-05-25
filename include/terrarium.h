#ifndef TERRARIUM_H
#define TERRARIUM_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : TP
* Created On : 17-2-2021
* File : wifi.h
***************************************************************/
/*****************
    Includes
******************/
#include <stdint.h>
#include <WiFiNINA.h>
#include <TimeLib.h>
/*****************
    Defines
******************/
// EEPROM has 256 bytes.
// Each timer   ( 9 bytes)
// Each ruleset (33 bytes)
// Sprayer rule is 14 bytes
#define NR_OF_RULESETS   2  // 2 x 33 bytes = 66 bytes
#define MAX_NR_OF_TIMERS 18  // 250 - 66(rules) - 14(sprayer) = 170 / 9 = max 18 timers
// All pins on Arduino Uno Wifi Rev2
#define pin_serial_tx    0
#define pin_serial_rx    1
#define pin_light1       2
#define pin_light2       3
#define pin_light3       4
#define pin_light4       5
#define pin_light5       6
#define pin_light6       7
#define pin_fan_out     10
#define pin_fan_in      11
#define pin_sprayer     12
#define pin_mist        13
#define pin_pump        14

#define pin_sensor_out  16
#define pin_sensor_in   17
#define lcd_sda         18
#define lcd_scl         19
// Devices
#define LIGHT1          0
#define LIGHT2          1
#define LIGHT3          2
#define LIGHT4          3
#define LIGHT5          4
#define LIGHT6          5
#define SPRAYER         6
#define MIST            7
#define PUMP            8
#define FAN_IN          9
#define FAN_OUT        10
#define NR_OF_DEVICES  11

/*****************
    Structs
******************/

typedef struct {
    char *name;
    int8_t pin_nr;
    int8_t nr_of_timers;
    int32_t end_time; // on, endtime in seconds since 2000-01-01 or -1 = until ideal value is reached, -2 = endless, off = 0
    int8_t temprule; // true/1: device is on because of a temperature rule, false/0: on because of a timer setting
    int8_t lcc; // true: lifecycle of this device is counted
    int32_t on_time; // counting the total number of seconds the device was on
    bool manual; // true = device is manually controlled.
} Device;

/*************************
    Function templates
*************************/
void gen_initEEPROM();
void gen_init();
void gen_setup();
Device *gen_getDevices();
int8_t gen_getDeviceIndex(char *device);
bool gen_isTraceOn();
void gen_setTraceOn(bool on);
void gen_getProperties(char *json);
void gen_getDeviceStates(char *json);
bool gen_isDeviceOn(int8_t device);
int32_t gen_getEndTime(int8_t device);
int8_t gen_isSetByRule(int8_t device);
void gen_setDeviceToManual(int8_t device, bool yes);
bool gen_isDeviceOnManual(int8_t device);
void gen_setDeviceState(char *devurl);
void gen_setDeviceState(int8_t device, int32_t end_time, int8_t temprule);
void gen_checkDeviceStates(time_t curtime);
void gen_showState(char *txt, int8_t device);
void gen_increase_time_on();
void gen_setCounter(char *devurl);

#endif /* TERRARIUM_H */