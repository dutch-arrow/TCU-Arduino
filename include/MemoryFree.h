#ifndef MEMORYFREE_H
#define MEMORYFREE_H
/**************************************************************
*
* Copyright © 2021 Dutch Arrow Software - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the Apache Software License 2.0.
*
* Author : Tom Pijl
* Created On : 6-3-2021
* File : MemoryFree.h
***************************************************************/
// MemoryFree library based on code posted here:
// https://forum.arduino.cc/index.php?topic=27536.msg204024#msg204024
// Extended by Matthew Murdoch to include walking of the free list.

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
#ifdef __cplusplus
extern "C" {
#endif

int freeMemory();

#ifdef  __cplusplus
}
#endif

#endif /* MEMORYFREE_H */

