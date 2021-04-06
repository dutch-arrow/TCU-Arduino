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
#ifdef TOM
char SSID[] = "ASUS-fampijl";
char PASSWORD[] = "arrowfamily2014";
#else
char SSID[] = "Familiepijl";
char PASSWORD[] = "Arrow6666!";
#endif
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
int8_t wifi_connect() {
	char tmp[100];
	logline("Entering wifi_connect()...");
	if (WiFi.status() == WL_NO_MODULE) {
		return 1;
	}
	// attempt to connect to Wifi network:
	int attempt = 0;
	while (status != WL_CONNECTED && attempt < 10) {
		attempt++;
		logline("Attempt %d ...", attempt);
		WiFi.disconnect();
		delay(1000);
		status = WiFi.begin(SSID, keyIndex, PASSWORD);
		logline("Wifi connection error: %d", status);
		// wait 10 seconds for connection to be made
		delay(10000);
	}
	if (status != WL_CONNECTED && attempt == 10) {
		return 2;
	}
	if (status == WL_CONNECTED) {
		return 0;
	}
}

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
int8_t wifi_init() {
	int8_t rc = wifi_connect();
	if (rc == 0) {
		char ip[16];
		wifi_getIPaddress(ip);
		logline("You're connected to the network with IP-address: %s", ip);
	}
	return rc;
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
		} else{
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