#ifndef WIFI_H
#define WIFI_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : TP
* Created On : 18-2-2021
* File : wifi.h
***************************************************************/
#ifndef SIMULATION
#include <Arduino.h>
#include <IPAddress.h>
#endif
/*****************
    Defines
******************/

/*****************
    Structs
******************/

/*************************
    Function templates
*************************/
/*
 * Connect to the local WIFI network and start a webserver
 * 
 * return: 0 = connection is made
 *         1 = Wifi module is broken
 *         2 = cannot connect to network after 10 tries
*/
int8_t wifi_init(char *ssid, char *password);

/*
 * Check if we are still connected to the local network
 * 
 * return: true or false
*/
bool wifi_isConnected();

/*
 * Set the RTC clock by requesting the time from the worldtimeapi.org site.
 * (http://worldtimeapi.org/api/timezone/Europe/Amsterdam)
 * 
 * return value: true, if the time was set, false if not
 */
int8_t wifi_setRTC();

void wifi_getIPaddress(char *ipstr);

#endif /* WIFI_H */
