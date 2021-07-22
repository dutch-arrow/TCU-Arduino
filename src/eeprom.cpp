/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 3-3-2021
* File : eeprom.cpp
***************************************************************/

/*****************
    Includes
******************/
#include <stdint.h>
#include <EEPROM.h>
#include "logger.h"
#include "terrarium.h"
#include "rules.h"
#include "timers.h"

/*****************
    Private data
******************/
#define ADDRESS_NR_OF_TIMERS 0
#define ADDRESS_START_ADDRESS_TIMERS 1
#define ADDRESS_NR_OF_RULESETS 2
#define ADDRESS_START_ADDRESS_RULESETS 3
#define ADDRESS_START_ADDRESS_SPRAYER_RULE 4

int8_t timerSize;
int8_t rulesetSize;
int8_t sprayerRuleSize;

/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
void epr_init() {
	timerSize = 9;	  //sizeof(Timer);
	rulesetSize = 33; //sizeof(RuleSet);
	sprayerRuleSize = 13; //sizeof(SprayerRule)
#ifdef INIT_EEPROM
    for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
    }
    logline("EEPROM memory is cleared");
#endif
}
int8_t epr_getNrOfTimersStored() {
	return EEPROM.read(ADDRESS_NR_OF_TIMERS);
}
int8_t epr_getNrOfRulesetsStored() {
	return EEPROM.read(ADDRESS_NR_OF_RULESETS);
}
void epr_setNrOfTimersStored(int8_t nr) {
	EEPROM.update(ADDRESS_NR_OF_TIMERS, nr);
	EEPROM.update(ADDRESS_START_ADDRESS_TIMERS, 5);
}
void epr_setNrOfRulesetsStored(int8_t nr) {
    EEPROM.update(ADDRESS_NR_OF_RULESETS, nr);
    EEPROM.update(ADDRESS_START_ADDRESS_RULESETS, 167);
    EEPROM.update(ADDRESS_START_ADDRESS_SPRAYER_RULE, 233);
}
void epr_saveTimerToEEPROM(int8_t i, Timer *t) {
	uint8_t ix = EEPROM.read(ADDRESS_START_ADDRESS_TIMERS);
	ix += timerSize * i;
	EEPROM.put(ix, *t);
}
void epr_getTimerFromEEPROM(int8_t i, Timer *t) {
    uint8_t ix = EEPROM.read(ADDRESS_START_ADDRESS_TIMERS);
    ix += timerSize * i;
    EEPROM.get(ix, *t);
}
void epr_saveRulesetToEEPROM(int8_t i, RuleSet *rs) {
    uint8_t ix = EEPROM.read(ADDRESS_START_ADDRESS_RULESETS);
	ix += rulesetSize * i;
	EEPROM.put(ix, *rs);
}
void epr_getRulesetFromEEPROM(int8_t i, RuleSet *rs) {
    uint8_t ix = EEPROM.read(ADDRESS_START_ADDRESS_RULESETS);
	ix += rulesetSize * i;
	EEPROM.get(ix, *rs);
}
void epr_saveSprayerRuleToEEPROM(SprayerRule *sr) {
    uint8_t ix = EEPROM.read(ADDRESS_START_ADDRESS_SPRAYER_RULE);
	EEPROM.put(ix, *sr);
}
void epr_getSprayerRuleFromEEPROM(SprayerRule *sr) {
    uint8_t ix = EEPROM.read(ADDRESS_START_ADDRESS_SPRAYER_RULE);
	EEPROM.get(ix, *sr);
}
void epr_clearHoursOn() {
	int32_t value = 0;
    EEPROM.put(248, value);
    EEPROM.put(252, value);
}
void epr_setHoursOn(int32_t nrOfHours) {
    int32_t value;
    EEPROM.get(248, value);
    EEPROM.put(248, value + 1); // Increase nr of EEPROM writes
    EEPROM.put(252, nrOfHours); // Set counter
}
void epr_decreaseHoursOn(int32_t nrOfHours) {
	int32_t value;
    EEPROM.get(248, value);
    EEPROM.put(248, value + 1); // Increase nr of EEPROM writes
	EEPROM.get(252, value);
    EEPROM.put(252, value - nrOfHours); // Decrease counter
}
int32_t epr_getHoursOn() {
	int32_t value;
	EEPROM.get(252, value);
	return value;
}
uint32_t epr_getEEPROMWriteCounter() {
    uint32_t value;
    EEPROM.get(248, value);
    return value;
}
