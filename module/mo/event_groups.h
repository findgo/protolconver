
#ifndef __EVENT_GROUPS_H__
#define __EVENT_GROUPS_H__

/* FreeRTOS includes. */
#include "timers.h"

#ifdef __cplusplus
extern "C" {
#endif

// 事件位掩码
typedef uint32_t EventBits_t;
// 句柄
typedef void * EventGroupHandle_t;
//静态结构体,用于屏蔽用户对结构体的可见
typedef struct EventGroupStatic_s
{
    uint32_t xDummy0;
} EventGroupStatic_t;

EventGroupHandle_t eventGroupNew( void ) ;
EventGroupHandle_t eventGroupAssign( EventGroupStatic_t *pxEventGroupBuffer ) ;

EventBits_t eventGroupWaitBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits, TickType_t xTicksToWait ) ;
EventBits_t eventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear ) ;
EventBits_t eventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet ) ;

#define xEventGroupGetBits( xEventGroup ) xEventGroupClearBits( xEventGroup, 0 )

#ifdef __cplusplus
}
#endif

#endif /* EVENT_GROUPS_H */



