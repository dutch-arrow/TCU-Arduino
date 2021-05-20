#ifndef RULES_H
#define RULES_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 28-2-2021
* File : rules.h
***************************************************************/

/*****************
    Includes
******************/
#include <TimeLib.h>
#include <stdint.h>
/*****************
    Defines
******************/

/*****************
    Structs
******************/

typedef struct {
    int8_t device;          // -1 = no device
    int16_t on_period;      // 0 = off, -1 = on, endless, -2 = on, until ideal value is reached, > 0 = on for 1-3600 seconds
} Action;

typedef struct { // size in bytes: 1 + 4 * 3 = 13
    int8_t value;           // positive = if above value, negative = if below value
    Action actions[4];
} Rule;

typedef struct { // size in bytes: 7 + 2 * 13 = 33
	int8_t terrarium_nr;
	bool active;
	int16_t from; // 1-1440 minutes
	int16_t to;   // 1-1440 minutes
	int8_t temp_ideal;
	Rule rules[2];
} RuleSet;

typedef struct {
	int8_t  delay;          // in minutes,max 255
    Action actions[4];
} SprayerRule;

/*************************
    Function templates
*************************/
void rls_initEEPROM();
void rls_init();

void rls_setSprayerRuleFromJson(char *json);
void rls_getSprayerRuleAsJson(char *json);
void rls_startSprayerRule(time_t curtime);
bool rls_isSprayerRuleActive();
void rls_checkSprayerRule(time_t curtime);

void rls_setRuleSetFromJson(int8_t setnr, char *json);
void rls_getRuleSetAsJson(int8_t setnr, char *json);
void rls_checkTempRules(time_t curtime);

void rls_switchRulesetsOff(void);
void rls_switchRulesetsOn(void);

#endif /* RULES_H */
