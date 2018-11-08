

#ifndef TIMERS_H
#define TIMERS_H

#include "common_type.h"
#include "memalloc.h"
#include "list.h"
#include "queue.h"
#include "mclock.h"

#define configTIMER_QUEUE_LENGTH  10

#ifdef __cplusplus
extern "C" {
#endif

/* IDs for commands that can be sent/received on the timer queue.*/
#define tmrCOMMAND_START                        ( ( uint32_t ) 0 )
#define tmrCOMMAND_STOP                         ( ( uint32_t ) 1 )
#define tmrCOMMAND_DELETE                       ( ( uint32_t ) 2 )


//静态结构体,用于屏蔽用户对结构体的可见
typedef struct TimerStatic_s
{
    ListItemStatic_t    xDummy0;
    void                *pvDummy1[2];
} TimerStatic_t;

/* timer handle */
typedef void * TimerHandle_t;
/* Defines the prototype to which timer callback functions must conform. */
typedef void (*TimerCallbackFunction_t)( void* arg );

#if( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
TimerHandle_t timerNew( TimerCallbackFunction_t pxCallbackFunction, void * arg) ;
#endif
TimerHandle_t timerAssign(TimerStatic_t * pxTimerBuffer, TimerCallbackFunction_t pxCallbackFunction, void * arg) ; 
uint8_t timerIsTimerActive( TimerHandle_t xTimer );
uint32_t timerGetNextTimeout(void);
void timerTask( void );

#define timerStart( xTimer, timeout)    timerGenericCommandSend( ( xTimer ), tmrCOMMAND_START, timeout)
#define timerRestart( xTimer, timeout ) timerStart( xTimer, timeout)
#define timerStop( xTimer )             timerGenericCommandSend( ( xTimer ), tmrCOMMAND_STOP, 0U )
#define timerDelete( xTimer )           timerGenericCommandSend( ( xTimer ), tmrCOMMAND_DELETE, 0U)

#define timerStartFromISR( xTimer, timeout)    timerStart( xTimer, timeout)
#define timerRestartFromISR( xTimer, timeout ) timerStart( xTimer, timeout)
#define timerStopFromISR( xTimer )             timerStop( xTimer )
#define timerDeleteFromISR( xTimer )           timerDelete( xTimer )

// internal function do not use it
uint8_t timerGenericCommandSend( TimerHandle_t xTimer, const uint32_t xCommandID, const uint32_t xTimeoutInTicks);

#ifdef __cplusplus
}
#endif
#endif /* TIMERS_H */



