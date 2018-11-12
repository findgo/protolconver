
/*
 *
 *
 */

#ifndef __PORTING_DEF_H__
#define __PORTING_DEF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


/* set support dynamic alloca compatibility. */
#define configSUPPORT_DYNAMIC_ALLOCATION        (1)
//使能事件触发
#define configSUPPORT_TASKS_EVENT               (0)


#define mtCOVERAGE_TEST_DELAY()
#define mtCOVERAGE_TEST_MARKER()

#endif
