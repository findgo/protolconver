
#ifndef __EVENT_GROUPS_H__
#define __EVENT_GROUPS_H__

#include "common_type.h"
#include "common_def.h"
#include "memalloc.h"
#include "port_atom.h"

#ifdef __cplusplus
extern "C" {
#endif


#define EVENTGROUP_BITS_0  (( uint32_t )1 << 0) 
#define EVENTGROUP_BITS_1  (( uint32_t )1 << 1) 
#define EVENTGROUP_BITS_2  (( uint32_t )1 << 2) 
#define EVENTGROUP_BITS_3  (( uint32_t )1 << 3) 
#define EVENTGROUP_BITS_4  (( uint32_t )1 << 4) 
#define EVENTGROUP_BITS_5  (( uint32_t )1 << 5) 
#define EVENTGROUP_BITS_6  (( uint32_t )1 << 6) 
#define EVENTGROUP_BITS_7  (( uint32_t )1 << 7) 
#define EVENTGROUP_BITS_8  (( uint32_t )1 << 8) 
#define EVENTGROUP_BITS_9  (( uint32_t )1 << 9) 
#define EVENTGROUP_BITS_10  (( uint32_t )1 << 10) 
#define EVENTGROUP_BITS_11  (( uint32_t )1 << 11) 
#define EVENTGROUP_BITS_12  (( uint32_t )1 << 12) 
#define EVENTGROUP_BITS_13  (( uint32_t )1 << 13) 
#define EVENTGROUP_BITS_14  (( uint32_t )1 << 14) 
#define EVENTGROUP_BITS_15  (( uint32_t )1 << 15) 
#define EVENTGROUP_BITS_16  (( uint32_t )1 << 16) 
#define EVENTGROUP_BITS_17  (( uint32_t )1 << 17) 
#define EVENTGROUP_BITS_18  (( uint32_t )1 << 18) 
#define EVENTGROUP_BITS_19  (( uint32_t )1 << 19) 
#define EVENTGROUP_BITS_20  (( uint32_t )1 << 20) 
#define EVENTGROUP_BITS_21  (( uint32_t )1 << 21) 
#define EVENTGROUP_BITS_22  (( uint32_t )1 << 22) 
#define EVENTGROUP_BITS_23  (( uint32_t )1 << 23) 
#define EVENTGROUP_BITS_24  (( uint32_t )1 << 24) 
#define EVENTGROUP_BITS_25  (( uint32_t )1 << 25) 
#define EVENTGROUP_BITS_26  (( uint32_t )1 << 26) 
#define EVENTGROUP_BITS_27  (( uint32_t )1 << 27) 
#define EVENTGROUP_BITS_28  (( uint32_t )1 << 28) 
#define EVENTGROUP_BITS_29  (( uint32_t )1 << 29) 
#define EVENTGROUP_BITS_30  (( uint32_t )1 << 30) 
#define EVENTGROUP_BITS_31  (( uint32_t )1 << 31) 


// 事件位掩码
typedef uint32_t EventBits_t;
// 句柄
typedef void * EventGroupHandle_t;
//静态结构体,用于屏蔽用户对结构体的可见
typedef struct EventGroupStatic_s
{
    uint32_t xDummy0;
} EventGroupStatic_t;


#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1)
EventGroupHandle_t eventGroupNew( void );
#endif
EventGroupHandle_t eventGroupAssign( EventGroupStatic_t *pxEventGroupBuffer ) ;

EventBits_t eventGroupWaitBits( EventGroupHandle_t xEventGroup, const EventBits_t uBitsToProcessFor);

EventBits_t eventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear ) ;
EventBits_t eventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet ) ;
EventBits_t eventGroupClearBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear );
EventBits_t eventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet );

#define eventGroupGetBits( xEventGroup ) eventGroupClearBits( xEventGroup, 0 )
#define eventGroupGetBitsFromISR( xEventGroup ) eventGroupClearBitsFromISR( xEventGroup, 0 )


#ifdef __cplusplus
}
#endif

#endif /* EVENT_GROUPS_H */



