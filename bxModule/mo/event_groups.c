
#include "event_groups.h"

typedef struct EventGroup_s
{
    EventBits_t eventBits;
} EventGroupInner_t;

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1)
EventGroup_t * eventGroupNew( void )
{
    EventGroupInner_t *pxEventBits;

    /* Allocate the event group. */
    pxEventBits = ( EventGroupInner_t * ) mo_malloc( sizeof( EventGroupInner_t ) );

    if( pxEventBits){
        pxEventBits->eventBits = 0;
    }

    return ( EventGroup_t * ) pxEventBits;
}
#endif

void eventGroupAssign( EventGroup_t *pxEventGroupBuffer )
{
    EventGroupInner_t *pxEventBits;

    /* A StaticEventGroup_t object must be provided. */
    configASSERT( pxEventGroupBuffer );

    /* The user has provided a statically allocated event group - use it. */
    pxEventBits = ( EventGroupInner_t * ) pxEventGroupBuffer; 

    if( pxEventBits ){
        pxEventBits->eventBits = 0;
    }
}

/*-----------------------------------------------------------*/

EventBits_t eventGroupWaitBits( EventGroup_t * xEventGroup, const EventBits_t uBitsToProcessFor)
{
    EventGroupInner_t *pxEventBits = ( EventGroupInner_t * ) xEventGroup;
    EventBits_t previousEventBits;

    /* Check the user is not attempting to wait on the bits used by the kernel
       itself, and that at least one bit is being requested. */
    configASSERT( xEventGroup );
    
    taskENTER_CRITICAL();
    // wait for all bit
    if( uBitsToProcessFor == ( EventBits_t )0 ){
        previousEventBits = pxEventBits->eventBits;
        pxEventBits->eventBits = 0; // clear all bit
    }
    else {
        previousEventBits = pxEventBits->eventBits & uBitsToProcessFor;
        pxEventBits->eventBits &= ~uBitsToProcessFor;
    
    }
    taskEXIT_CRITICAL();
    
    return previousEventBits;
}

EventBits_t eventGroupClearBits( EventGroup_t * xEventGroup, const EventBits_t uxBitsToClear )
{
    EventBits_t uxReturn;
    isrSaveCriticial_status_Variable;

    /* Check the user is not attempting to clear the bits used by the kernel itself. */
    configASSERT( xEventGroup );

    isrENTER_CRITICAL();
    /* The value returned is the event group value prior to the bits being cleared. */
    uxReturn = ( ( EventGroupInner_t * ) xEventGroup )->eventBits;

    /* Clear the bits. */
    ( ( EventGroupInner_t * ) xEventGroup )->eventBits &= ~uxBitsToClear;
    isrEXIT_CRITICAL();
    
    return uxReturn;
}


EventBits_t eventGroupSetBits( EventGroup_t * xEventGroup, const EventBits_t uxBitsToSet )
{
    EventBits_t uxReturn;
    isrSaveCriticial_status_Variable;
    
    /* Check the user is not attempting to set the bits used by the kernel itself. */
    configASSERT( xEventGroup );
    
    isrENTER_CRITICAL();
    uxReturn = ( ( EventGroupInner_t * ) xEventGroup )->eventBits;
    /* Set the bits. */
    ( ( EventGroupInner_t * ) xEventGroup )->eventBits |= uxBitsToSet;
    isrEXIT_CRITICAL();

    return uxReturn;
}
