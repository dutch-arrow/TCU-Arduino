/**************************************************************
 *
 * Copyright © 2021 Dutch Arrow Software - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the Apache Software License 2.0.
 *
 * Author : TP
 * Created On : Wed Feb 17 2021
 * File : wifi.cpp
 ***************************************************************/

/*****************
    Includes
******************/
#include "config.h"
#include "terrarium.h"
#include "rtc.h"
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include "MemoryFree.h"

/*****************
    Private data
******************/

/**********************
    Private functions
**********************/

/*****************************************************************
    Public functions (templates in the corresponding header-file)
******************************************************************/
void logline(char *format, ...) {
	if (gen_isTraceOn()) {
		time_t curtime = rtc_now();
		char tmp[250];
		sprintf(tmp, "%02d:%02d:%02d ", rtc_hour(curtime), rtc_minute(curtime), rtc_second(curtime));
		Serial1.print(tmp);
		va_list l_Arg;
		va_start(l_Arg, format);
		vsprintf(tmp, format, l_Arg);
		Serial1.println(tmp);
		Serial1.flush();
	}
}
