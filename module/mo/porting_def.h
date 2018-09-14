

// 本文件主要是从其它平台移植的库的一些定义


#ifndef __PORTING_DEF_H__
#define __PORTING_DEF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

//from freeRTOS 
//use for list heap 

// macro define by use
typedef uint32_t TickType_t;
typedef unsigned long UBaseType_t;
typedef long BaseType_t;

#define pdFALSE         ( ( BaseType_t ) 0 )
#define pdTRUE          ( ( BaseType_t ) 1 )

#define portMAX_DELAY  ( TickType_t )0xffffffffUL
#define mtCOVERAGE_TEST_DELAY()
#define mtCOVERAGE_TEST_MARKER()
#define configASSERT( x )



#endif
