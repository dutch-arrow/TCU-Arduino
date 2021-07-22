/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : TP
* Created On : 17-2-2021
* File : wifi.cpp
***************************************************************/

/*****************
    Includes
******************/
#ifndef SIMULATION
#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <TimeLib.h>
#endif
#include <JsonParser.h>
#include "wifi.h"
#include "config.h"
#include "logger.h"
#include "terrarium.h"

/*****************
    Private data
******************/
int keyIndex = 0; // my network key Index number
int status = WL_IDLE_STATUS;

/**********************
    Private functions
**********************/
/*
 * Connect to the local Wifi network.
 * return value: 0 = connection is made
 *               1 = Wifi module is broken
 *               2 = cannot connect to network after 10 tries
 */
int8_t wifi_connect(char *ssid, char *password) {
	logline("Trying to connect to Wifi network...");
	WiFi.disconnect();
	delay(1000);
	status = WiFi.begin(ssid, keyIndex, password);
	/*
        WL_IDLE_STATUS     = 0
        WL_NO_SSID_AVAIL   = 1
        WL_SCAN_COMPLETED  = 2
        WL_CONNECTED       = 3
        WL_CONNECT_FAILED  = 4
        WL_CONNECTION_LOST = 5
        WL_DISCONNECTED    = 6
        WL_AP_LISTENING    = 7
        WL_AP_CONNECTED    = 8
        WL_AP_FAILED       = 9
	*/
	if (status == WL_CONNECTED) {
		return 0;
	} else {
		logline("Wifi status = %d", status);
		return status;
	}
}

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
int8_t wifi_init(char *ssid, char *password) {
	return wifi_connect(ssid, password);
}

int8_t wifi_setRTC() {
	logline("Setting date and time...");
	WiFiClient client;
	client.stop();
	if (client.connect("worldtimeapi.org", 80)) {
		client.print("GET /api/timezone/Europe/Amsterdam HTTP/1.1\r\nHost: worldtimeapi.org\r\nConnection: close\r\n\r\n");
		while (!client.available() && client.connected()) {
			delay(500);
		}
		if (client.available() && client.connected()) {
			client.find("\r\n\r\n");
/*
			{
				"abbreviation": "CET",
				"client_ip": "217.104.121.100",
				"datetime": "2021-03-13T18:49:20.897959+01:00",
				"day_of_week": 6,
				"day_of_year": 72,
				"dst": false,
				"dst_from": null,
				"dst_offset": 0,
				"dst_until": null,
				"raw_offset": 3600,
				"timezone": "Europe/Amsterdam",
				"unixtime": 1615657760,
				"utc_datetime": "2021-03-13T17:49:20.897959+00:00",
				"utc_offset": "+01:00",
				"week_number": 10
			}
*/
			char json[450];
			strncpy(json, client.readString().c_str(), 450);
			JsonParser<35> parser;
			JsonHashTable tm = parser.parseHashTable(json);
			if (!tm.success()) {
				logline("setRTC() -> deserializeJson() failed");
				return -2;
			}
			long unixtime = tm.getLong("unixtime");
			String offset = tm.getString("utc_offset");
			setTime(unixtime + (offset.substring(2, 3).toInt() * 3600));
			logline("Date and time is synced.");
		} else {
			return -1;
		}
		if (client.connected()) {
			client.stop();
		}
		return 0;
	} else {
		return -1;
	}
}

bool wifi_isConnected() {
	return WiFi.status() == WL_CONNECTED;
}

void wifi_getIPaddress(char *ipstr) {
	IPAddress ip = WiFi.localIP();
	sprintf(ipstr, "%3d.%3d.%3d.%3d", ip[0], ip[1], ip[2], ip[3]);
}