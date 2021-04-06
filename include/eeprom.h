#ifndef EEPROM_H
#define EEPROM_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 3-3-2021
* File : eeprom.h
***************************************************************/

/*****************
    Includes
******************/
#include <stdint.h>
#include "rules.h"
#include "timers.h"

/*****************
    Defines
******************/

/*****************
    Structs
******************/

/*************************
    Function templates
*************************/
void epr_init();
int8_t epr_getNrOfTimersStored();
int8_t epr_getNrOfRulesetsStored();
void epr_setNrOfTimersStored(int8_t nr);
void epr_setNrOfRulesetsStored(int8_t nr);
void epr_saveTimerToEEPROM(int8_t i, Timer *t);
void epr_getTimerFromEEPROM(int8_t i, Timer *t);
void epr_saveRulesetToEEPROM(int8_t i, RuleSet *rs);
void epr_getRulesetFromEEPROM(int8_t i, RuleSet *rs);
void epr_saveSprayerRuleToEEPROM(SprayerRule *sr);
void epr_getSprayerRuleFromEEPROM(SprayerRule *sr);
void epr_clearHoursOn();
void epr_setHoursOn(int32_t nrOfHours);
void epr_decreaseHoursOn(int32_t nrOfHours);
int32_t epr_getHoursOn();
uint32_t epr_getEEPROMWriteCounter();

#endif /* EEPROM_H */
