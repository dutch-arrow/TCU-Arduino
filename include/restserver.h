#ifndef RESTSERVER_H
#define RESTSERVER_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 25-2-2021
* File : restserver.h
***************************************************************/

/*****************
    Includes
******************/

/*****************
    Defines
******************/
#define GET     1
#define PUT     2
#define POST    3
#define DELETE  4

/*****************
    Structs
******************/

/*************************
    Function templates
*************************/
bool restserver_init();
void restserver_handle_request();
bool isRestserverListening();

#endif /* RESTSERVER_H */
