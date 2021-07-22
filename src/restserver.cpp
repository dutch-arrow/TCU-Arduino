/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 25-2-2021
* File : restserver.cpp
***************************************************************/

/*****************
    Includes
******************/
#include <TimeLib.h>
#include <WiFiNINA.h>
#include "restserver.h"
#include "logger.h"
#include "rtc.h"
#include "rules.h"
#include "sensors.h"
#include "terrarium.h"
#include "wifi.h"
#include "timers.h"

/*****************
    Private data
******************/
WiFiServer server(80);
WiFiClient client;
char jsonString[1300];

/**********************
    Private functions
**********************/

void sendResponse(char *jsonString) {
	client.println("HTTP/1.1 200 OK");
	client.println("Content-Type: application/json");
	client.println("Connection: close");
	client.print("Content-Length: ");
	client.println(strlen(jsonString));
	client.println();
	client.println(jsonString);
}

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
bool restserver_init() {
	if (wifi_isConnected()) {
		// start the webserver
		server.begin();
		if (server.status() != LISTEN) {
			logline("REST server is not started.");
			return false;
		}
	} else {
		logline("Cannot start REST server: no local network connection.");
		return false;
	}
	logline("REST server started.");
	return true;
}

void restserver_handle_request() {
	if (server.status() == LISTEN) {
		// listen for incoming clients
		client = server.available();
		if (client) {
			// an http request line ends with a blank line
			// read the request line
			char req[50] = {0};
			client.readBytesUntil('\r', req, 50);
			// req=[method] [url] HTTP 1.1 ....
			*strstr(req, " HTTP") = 0;
			// req=[method] [url]
			logline("Request: %s", req);
			if (strcmp(req, "GET /properties") == 0) {
				gen_getProperties(jsonString);
			} else if (strcmp(req, "GET /sensors") == 0) {
				sensors_tojson(jsonString);
			} else if (strcmp(req, "GET /state") == 0) {
				gen_getDeviceStates(jsonString);
			} else if (strncmp(req, "PUT /device", 11) == 0) {
				gen_setDeviceState(req + 12);
				jsonString[0] = 0;
			} else if (strncmp(req, "GET /ruleset", 12) == 0) {
				rls_getRuleSetAsJson(atoi(req + 13) - 1, jsonString);
			} else if (strncmp(req, "PUT /ruleset", 12) == 0) {
				client.find("\r\n\r\n");
				int sz = client.readBytes(jsonString, 1300);
				jsonString[sz] = '\0';
				Serial1.println(jsonString);
				rls_setRuleSetFromJson(atoi(req + 13) - 1, jsonString);
			} else if (strncmp(req, "GET /sprayerrule", 16) == 0) {
				rls_getSprayerRuleAsJson(jsonString);
			} else if (strncmp(req, "PUT /sprayerrule", 16) == 0) {
				client.find("\r\n\r\n");
				int sz = client.readBytes(jsonString, 1300);
				jsonString[sz] = '\0';
				Serial1.println(jsonString);
				rls_setSprayerRuleFromJson(jsonString);
			} else if (strncmp(req, "GET /timers", 11) == 0) {
				tmr_getTimersAsJson(req + 12, jsonString);
			} else if (strncmp(req, "PUT /timers", 11) == 0) {
				client.find("\r\n\r\n");
				int sz = client.readBytes(jsonString, 1300);
				jsonString[sz] = '\0';
				Serial1.println(jsonString);
				tmr_setTimersFromJson(jsonString);
			} else if (strncmp(req, "POST /setdate", 13) == 0) {
				rtc_setTime(req + 14);
				jsonString[0] = 0;
			} else if (strncmp(req, "POST /trace/on", 14) == 0) {
				gen_setTraceOn(true);
				jsonString[0] = 0;
			} else if (strncmp(req, "POST /trace/off", 15) == 0) {
				gen_setTraceOn(false);
			} else if (strncmp(req, "POST /counter", 13) == 0) {
				gen_setCounter(req + 14);
				jsonString[0] = 0;
			} else if (strcmp(req, "POST /test/off") == 0) {
				sensors_setTestOff();
				jsonString[0] = 0;
			} else if (strncmp(req, "POST /test", 10) == 0) {
				sensors_setTestValues(req + 11);
				jsonString[0] = 0;
			}
			// create the response
			if (client.connected()) {
				sendResponse(jsonString);
				if (strlen(jsonString) > 0) {
					logline("Sent %d bytes:", strlen(jsonString));
					Serial1.println(jsonString);
				}
			}
			delay(2000);
			// close the connection:
			client.stop();
		}
	}
}

bool isRestserverListening() {
	return server.status() == LISTEN;
}

WiFiServer getServer() {
	return server;
}
