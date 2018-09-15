

#ifndef TIMERS_H
#define TIMERS_H

#include "common_type.h"
#include "heap_mange.h"
#include "list.h"
#include "queue.h"
#include "mclock.h"

#define configTIMER_QUEUE_LENGTH  10

#ifdef __cplusplus
extern "C" {
#endif

/* MACROS AND DEFINITIONS */

/* IDs for commands that can be sent/received on the timer queue.  These are to
be used solely through the macros that make up the public software timer API,
as defined below.  The commands that are sent from interrupts must use the
highest numbers as tmrFIRST_FROM_ISR_COMMAND is used to determine if the task
or interrupt version of the queue send function should be used. */
#define tmrCOMMAND_START                        ( ( uint32_t ) 0 )
#define tmrCOMMAND_STOP                         ( ( uint32_t ) 1 )
#define tmrCOMMAND_DELETE                       ( ( uint32_t ) 2 )

typedef struct TimerStatic_s
{
    ListItemStatic_t    xDummy0;
    void                *pvDummy1[2];
} TimerStatic_t;

/* timer handle */
typedef void * TimerHandle_t;
/* Defines the prototype to which timer callback functions must conform. */
typedef void (*TimerCallbackFunction_t)( void* arg );

TimerHandle_t timerNew( TimerCallbackFunction_t pxCallbackFunction, void * arg) ;
TimerHandle_t timerAssign(TimerStatic_t * pxTimerBuffer, TimerCallbackFunction_t pxCallbackFunction, void * arg) ; 
uint8_t timerIsTimerActive( TimerHandle_t xTimer ) ;
void timerTask( void ) ;
#define timerStart( xTimer, timeout)    timerGenericCommand( ( xTimer ), tmrCOMMAND_START, timeout)
#define timerRestart( xTimer, timeout ) timerStart( xTimer, timeout)
#define timerStop( xTimer )             timerGenericCommand( ( xTimer ), tmrCOMMAND_STOP, 0U )
#define timerDelete( xTimer )           timerGenericCommand( ( xTimer ), tmrCOMMAND_DELETE, 0U)

// internal function do not use it
uint8_t timerGenericCommand( TimerHandle_t xTimer, const uint32_t xCommandID, const uint32_t xTimeoutInTicks);

#ifdef __cplusplus
}
#endif
#endif /* TIMERS_H */



