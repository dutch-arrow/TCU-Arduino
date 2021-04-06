#ifndef SENSORS_H
#define SENSORS_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 25-2-2021
* File : sensors.h
***************************************************************/

/*****************
    Includes
******************/

/*****************
    Defines
******************/

/*****************
    Structs
******************/

/*************************
    Function templates
*************************/
void sensors_init();
void sensors_read();
void sensors_tojson(char *json);
// Getters
int8_t sensors_getRoomTemp();
int8_t sensors_getTerrariumTemp();
// Setters
void sensors_setTestValues(char *testurl);
void sensors_setTestOff();

#endif /* SENSORS_H */
