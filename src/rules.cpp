/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 28-2-2021
* File : rules.cpp
***************************************************************/

/*****************
    Includes
******************/
#ifndef SIMULATION
#include <JsonParser.h>
#include <TimeLib.h>
#endif
#include "eeprom.h"
#include "logger.h"
#include "rules.h"
#include "sensors.h"
#include "terrarium.h"

/*****************
    Private data
******************/

static RuleSet rulesets[] = {
    {0, false, 0, 0, 0, {{0, {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}}, {0, {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}}}},
    {1, false, 0, 0, 0, {{0, {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}}, {0, {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}}}}
};
static SprayerRule sprayerRule = {0, {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}};

bool sprayerRuleActive = false;
bool sprayerActionsExecuted = false;
time_t startTime, stopTime;
int16_t max_period = 0;
bool rulesetActive[2];
bool rulesetWasActive[2];

/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
void rls_initEEPROM() {
	epr_setNrOfRulesetsStored(NR_OF_RULESETS);
	for (int i = 0; i < NR_OF_RULESETS; i++) {
		epr_saveRulesetToEEPROM(i, &rulesets[i]);
	}
	epr_saveSprayerRuleToEEPROM(&sprayerRule);
	logline("EEPROM for rules initialized.");
}

void rls_init() {
	for (int i = 0; i < NR_OF_RULESETS; i++) {
		epr_getRulesetFromEEPROM(i, &rulesets[i]);
		rulesetActive[i] = rulesets[i].active;
	}
	epr_getSprayerRuleFromEEPROM(&sprayerRule);
	for (int i = 0; i < 4; i++) {
		if (sprayerRule.actions[i].device > 0) {
			max_period = sprayerRule.actions[i].on_period > max_period ? sprayerRule.actions[i].on_period : max_period;
		}
	}
	logline("Rules initialized.");
}
/*
{
    "delay": 15,
    "actions": [
        {"device": "fan_in", "on_period": 900},
        {"device": "fan_out","on_period": 900},
        {"device": "no device", "on_period": 0},
        {"device": "no device","on_period": 0}
    ]
}
*/
void rls_setSprayerRuleFromJson(char *json) {
	JsonParser<100> parser;
	JsonHashTable rl = parser.parseHashTable(json);
	if (!rl.success()) {
		// create the error response
		sprintf(json, "{\"error_msg\":\"Could not deserialize the JSON\"}");
		logline("deserializeJson() failed");
		return;
	} else {
		int delay = rl.getLong("delay");
		sprayerRule.delay = delay;
		JsonArray actions = rl.getArray("actions");
		for (int8_t i = 0; i < actions.getLength(); i++) {
			JsonHashTable action = actions.getHashTable(i);
			char *device = action.getString("device");
			int16_t on_period = action.getLong("on_period");
			logline("Sprayer action %d device='%s' on_period=%d", i, device, on_period);
			sprayerRule.actions[i].device =	gen_getDeviceIndex(device);
			sprayerRule.actions[i].on_period = on_period;
		}
		epr_saveSprayerRuleToEEPROM(&sprayerRule);
	}
	sprintf(json, "");
}

void rls_getSprayerRuleAsJson(char *json) {
	Device *devices = gen_getDevices();
	char tmp[200];
	sprintf(tmp, "{\"delay\":%d,\"actions\":[", sprayerRule.delay);
	strcpy(json, tmp);
	for (int8_t i = 0; i < 4; i++) {
		Action a = sprayerRule.actions[i];
		sprintf(tmp, "{\"device\":\"%s\",\"on_period\":%d}", a.device == -1 ? "no device" : devices[a.device].name, a.on_period);
		if (i != 3) {
			strcat(tmp, ",");
		}
		strcat(json, tmp);
	}
	strcat(json, "]}");
}

void rls_startSprayerRule(time_t curtime) {
	startTime = curtime + (sprayerRule.delay * 60);
	stopTime = startTime + max_period;
	sprayerRuleActive = true;
    sprayerActionsExecuted = false;
	rls_switchRulesetsOff();
	logline("Sprayer rule is activated");
}

bool rls_isSprayerRuleActive() { // used by the timer checks
	return sprayerRuleActive;
}

void rls_checkSprayerRule(time_t curtime) {
    logline("Check sprayer rule");
    if (sprayerRuleActive && curtime > startTime && !sprayerActionsExecuted) {
    	logline("  Sprayer rule actions are executed");
        // execute the actions
        for (int i = 0; i < 4; i++) {
            if (sprayerRule.actions[i].device > 0) {
                gen_setDeviceState(sprayerRule.actions[i].device,
                                   (curtime + sprayerRule.actions[i].on_period),
                                   0);
            }
        }
        sprayerActionsExecuted = true;
    } else if (sprayerRuleActive && curtime > stopTime && sprayerActionsExecuted) {
        sprayerRuleActive = false;
		// Make all rules that were active, active again
		rls_switchRulesetsOn();
    	logline("  Sprayer rule is not active anymore");
    } else if (!sprayerRuleActive) {
		logline("  Sprayer rule is not active");
	}
}

/*
{   "terrarium":1,"active":"yes","from":"10:30", "to":"22:15","temp_ideal":26, 
    "rules": [
        {"value":-25, "actions": [{"device":"fan1_in", "on_period":-2},{"device":"no device", "on_period":0},{"device":"no device", "on_period":0},{"device":"no device", "on_period":0}]},
        {"value":28, "actions": [{"device":"fan1_out", "on_period":-2},{"device":"no device", "on_period":0},{"device":"no device", "on_period":0},{"device":"no device", "on_period":0}]}
    ]}
}
*/
void rls_setRuleSetFromJson(int8_t setnr, char *json) {
	logline("Update ruleset %d", setnr);
	JsonParser<100> parser;
	JsonHashTable ruleset = parser.parseHashTable(json);
	if (!ruleset.success()) {
		// create the error response
		sprintf(json, "{\"error_msg\":\"Could not deserialize the JSON\"}");
		logline("deserializeJson() failed");
		return;
	} else {
		int8_t ruleset_terrarium = ruleset.getLong("terrarium");
		char *ruleset_active = ruleset.getString("active");
		char *ruleset_from = ruleset.getString("from");
		char *ruleset_to = ruleset.getString("to");
		int8_t ruleset_temp_ideal = ruleset.getLong("temp_ideal");
//		logline("terrarium %d, active %s, from %s, to %s, ideal %d", ruleset_terrarium, ruleset_active, ruleset_from, ruleset_to, ruleset_temp_ideal);
		rulesets[setnr].terrarium_nr = ruleset_terrarium;
		rulesets[setnr].active = strcmp(ruleset_active, "yes") == 0;
		rulesetActive[setnr] = rulesets[setnr].active;
		rulesets[setnr].from = atoi(strtok(ruleset_from, ":")) * 60 + atoi(strtok(NULL, ":"));
		rulesets[setnr].to = atoi(strtok(ruleset_to, ":")) * 60 + atoi(strtok(NULL, ":"));
		rulesets[setnr].temp_ideal = ruleset_temp_ideal;
		JsonArray rls = ruleset.getArray("rules");
		for (int8_t i = 0; i < rls.getLength(); i++) {
			JsonHashTable rule = rls.getHashTable(i);
			int8_t value = rule.getLong("value");
			rulesets[setnr].rules[i].value = value;
			JsonArray actions = rule.getArray("actions");
			for (int8_t j = 0; j < actions.getLength(); j++) {
				JsonHashTable action = actions.getHashTable(j);
				char *device = action.getString("device");
				int16_t on_period = action.getLong("on_period");
				rulesets[setnr].rules[i].actions[j].device = gen_getDeviceIndex(device);
				rulesets[setnr].rules[i].actions[j].on_period = on_period;
			}
		}
		sprintf(json, "");
		epr_saveRulesetToEEPROM(setnr, &rulesets[setnr]);
		logline("Ruleset %d for terrarium %d is updated.", setnr, rulesets[setnr].terrarium_nr);
	}
}

void rls_getRuleSetAsJson(int8_t setnr, char *json) {
	char tmp[100];
	RuleSet ruleset = rulesets[setnr];
	sprintf(tmp, "{\"terrarium\":%d, \"active\":\"%s\",", ruleset.terrarium_nr, ruleset.active ? "yes" : "no");
	strcpy(json, tmp);
	sprintf(tmp, "\"from\":\"%02d:%02d\",", ruleset.from / 60, ruleset.from - (ruleset.from / 60) * 60);
	strcat(json, tmp);
	sprintf(tmp, "\"to\":\"%02d:%02d\",", ruleset.to / 60, ruleset.to - (ruleset.to / 60) * 60);
	strcat(json, tmp);
	sprintf(tmp, "\"temp_ideal\":%d,\"rules\":[", ruleset.temp_ideal);
	strcat(json, tmp);
	for (int i = 0; i < 2; i++) { // 2 rules
		sprintf(tmp, "{\"value\":%d, \"actions\":[", ruleset.rules[i].value);
		strcat(json, tmp);
		for (int j = 0; j < 4; j++) { // 4 actions
			sprintf(tmp, "{\"device\":\"%s\",\"on_period\":%d}",
				(ruleset.rules[i].actions[j].device == -1 ? "no device" : gen_getDevices()[ruleset.rules[i].actions[j].device].name),
				ruleset.rules[i].actions[j].on_period);
			strcat(json, tmp);
			if (j != 3) {
				strcat(json, ",");
			} else {
				strcat(json, "]}");
			}
		}
		if (i != 1) {
			strcat(json, ",");
		} else {
			strcat(json, "]");
		}
	}
	strcat(json, "}");
}

void rls_performActions(Action *actions, int32_t curtime) {
	for (int a = 0; a < 4; a++) { // 4 actions per rule
		if (!gen_isDeviceOnManual(actions[a].device)) {
			if (actions[a].on_period != 0) { // so -2 (untill ideal value is reached) or >0. -1 (no endtime) is reserved for timers)
				if (curtime == 0) { // switch off
					gen_showState("switch off", actions[a].device);
					if (gen_getEndTime(actions[a].device) == -2) {
						gen_setDeviceState(actions[a].device, 0, 0);
					}
				} else { // switch on
					gen_showState("switch on ", actions[a].device);
					// if device is on, don't do anything
					if (gen_getEndTime(actions[a].device) == 0) {
						int16_t period = actions[a].on_period;
						int32_t endtime;
						if (actions[a].device != -1 && period > 0) {
							endtime = curtime + period;
							gen_setDeviceState(actions[a].device, endtime, 1);
						} else if (actions[a].device != -1 && period <= 0) {
							gen_setDeviceState(actions[a].device, period, 1);
						}
					}
				}
			}
		}
	}
}

void rls_checkTempRules(time_t curtime) {
    logline("Check other rules");
	int16_t curmins = rtc_hour(curtime) * 60 + rtc_minute(curtime);
	for (int rs = 0; rs < 2; rs++) { // 2 rulesets
		RuleSet rlst = rulesets[rs];
		if (rlst.active) {
			// rule is now active
			if (rlst.from > rlst.to) { // period is passing 00:00
				logline("  Check temperature rules of set %d, active period passing 00:00 hours", rs + 1);
				if ((curmins >= rlst.from || (curmins <= rlst.from && curmins < rlst.to))) {
					for (int r = 0; r < 2; r++) { // 2 rules per ruleset
						Rule rl = rlst.rules[r];
						if (rl.value < 0 && sensors_getTerrariumTemp() < -rl.value) {
							// perform actions
							rls_performActions(rl.actions, curtime);
						} else if (rl.value < 0 && sensors_getTerrariumTemp() >= rlst.temp_ideal) {
							// perform actions
							rls_performActions(rl.actions, 0);
						} else if (rl.value > 0 && sensors_getTerrariumTemp() > rl.value) {
							// perform actions
							rls_performActions(rl.actions, curtime);
						} else if (rl.value > 0 && sensors_getTerrariumTemp() <= rlst.temp_ideal) {
							// perform actions
							rls_performActions(rl.actions, 0);
						}
					}
				} else {
					for (int r = 0; r < 2; r++) { // 2 rules per ruleset
						Rule rl = rlst.rules[r];
						rls_performActions(rl.actions, 0);
					}
				}
			} else { // normal: from < to
				logline("  Check temperature rules of set %d, normal active period", rs + 1);
				logline("    from=%d curmins=%d to=%d", rlst.from, curmins, rlst.to);
				if (rlst.from <= curmins && rlst.to > curmins) {
					// ruleset is now active
					for (int r = 0; r < 2; r++) { // 2 rules per ruleset
						Rule rl = rlst.rules[r];
						logline("    temp=%d rlvalue=%d", sensors_getTerrariumTemp(), rl.value);
						if (rl.value < 0 && sensors_getTerrariumTemp() < -rl.value) {
							// perform actions
							rls_performActions(rl.actions, curtime);
						} else if (rl.value < 0 && sensors_getTerrariumTemp() >= rlst.temp_ideal) {
							// reset actions
							rls_performActions(rl.actions, 0);
						} else if (rl.value > 0 && sensors_getTerrariumTemp() > rl.value) {
							// perform actions
							rls_performActions(rl.actions, curtime);
						} else if (rl.value > 0 && sensors_getTerrariumTemp() <= rlst.temp_ideal) {
							// reset actions
							rls_performActions(rl.actions, 0);
						}
					}
				} else { // rulest is now not active
					for (int r = 0; r < 2; r++) { // 2 rules per ruleset
						Rule rl = rlst.rules[r];
						rls_performActions(rl.actions, 0);
					}
				}
			}
		} else { //ruleset is not active
			if (rulesetWasActive[rs]) {
				logline("  Undo temperature rules of inactive set %d", rs + 1);
				for (int r = 0; r < 2; r++) { // 2 rules per ruleset
					Rule rl = rlst.rules[r];
					// Undo all actions
					rls_performActions(rl.actions, 0);
				}
				rulesetWasActive[rs] = false;
			} else {
				logline("  Ruleset %d is not active", rs);
			}
		}
	}
}

void rls_switchRulesetsOff(void) {
	rulesetWasActive[0] = rulesetActive[0];
	rulesetWasActive[1] = rulesetActive[1];
	// Make all rules inactive
    rulesets[0].active = false;
    rulesets[1].active = false;
	logline("Rulesets are switched off");
}

void rls_switchRulesetsOn(void) {
	// Make all rules active
    rulesets[0].active = rulesetActive[0];
    rulesets[1].active = rulesetActive[1];
	logline("Rulesets are switched on");
}