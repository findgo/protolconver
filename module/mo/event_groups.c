
/* Standard includes. */
#include <stdlib.h>



#include "task.h"
#include "timers.h"
#include "event_groups.h"


/* The following bit fields convey control information in a task's event list item value.  
 * It is important they don't clash with the taskEVENT_LIST_ITEM_VALUE_IN_USE definition. */
#define eventCLEAR_EVENTS_ON_EXIT_BIT   0x01000000UL
#define eventUNBLOCKED_DUE_TO_BIT_SET   0x02000000UL
#define eventWAIT_FOR_ALL_BITS          0x04000000UL
#define eventEVENT_BITS_CONTROL_BYTES   0xff000000UL

typedef struct EventGroup_s
{
    EventBits_t uxEventBits;
    List_t xTasksWaitingForBits;        /*< List of tasks waiting for a bit to be set. */
} EventGroup_t;

/*-----------------------------------------------------------*/
EventGroupHandle_t eventGroupNew( void )
{
    EventGroup_t *pxEventBits;

    /* Allocate the event group. */
    pxEventBits = ( EventGroup_t * ) mo_malloc( sizeof( EventGroup_t ) );

    if( pxEventBits){
        pxEventBits->uxEventBits = 0;
    }

    return ( EventGroupHandle_t ) pxEventBits;
}

EventGroupHandle_t eventGroupAssign( EventGroupStatic_t *pxEventGroupBuffer )
{
    EventGroup_t *pxEventBits;

    /* A StaticEventGroup_t object must be provided. */
    configASSERT( pxEventGroupBuffer );

    /* The user has provided a statically allocated event group - use it. */
    pxEventBits = ( EventGroup_t * ) pxEventGroupBuffer; 

    if( pxEventBits ){
        pxEventBits->uxEventBits = 0;
    }

    return ( EventGroupHandle_t ) pxEventBits;
}

/*-----------------------------------------------------------*/

EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor, const BaseType_t xClearOnExit, const BaseType_t xWaitForAllBits)
{
    EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
    EventBits_t uxReturn, uxControlBits = 0;
    uint32_t xWaitConditionMet, xAlreadyYielded;
    uint8_t xTimeoutOccurred = FALSE;

    /* Check the user is not attempting to wait on the bits used by the kernel
    itself, and that at least one bit is being requested. */
    configASSERT( xEventGroup );
    configASSERT( ( uxBitsToWaitFor & eventEVENT_BITS_CONTROL_BYTES ) == 0 );
    configASSERT( uxBitsToWaitFor != 0 );

    vTaskSuspendAll();
    {
        const EventBits_t uxCurrentEventBits = pxEventBits->uxEventBits;

        /* Check to see if the wait condition is already met or not. */
        xWaitConditionMet = prvTestWaitCondition( uxCurrentEventBits, uxBitsToWaitFor, xWaitForAllBits );

        if( xWaitConditionMet != pdFALSE )
        {
            /* The wait condition has already been met so there is no need to
            block. */
            uxReturn = uxCurrentEventBits;
            xTicksToWait = ( TickType_t ) 0;

            /* Clear the wait bits if requested to do so. */
            if( xClearOnExit != pdFALSE )
            {
                pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else if( xTicksToWait == ( TickType_t ) 0 )
        {
            /* The wait condition has not been met, but no block time was
            specified, so just return the current value. */
            uxReturn = uxCurrentEventBits;
        }
        else
        {
            /* The task is going to block to wait for its required bits to be
            set.  uxControlBits are used to remember the specified behaviour of
            this call to xEventGroupWaitBits() - for use when the event bits
            unblock the task. */
            if( xClearOnExit != pdFALSE )
            {
                uxControlBits |= eventCLEAR_EVENTS_ON_EXIT_BIT;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }

            if( xWaitForAllBits != pdFALSE )
            {
                uxControlBits |= eventWAIT_FOR_ALL_BITS;
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }

            /* Store the bits that the calling task is waiting for in the
            task's event list item so the kernel knows when a match is
            found.  Then enter the blocked state. */
            vTaskPlaceOnUnorderedEventList( &( pxEventBits->xTasksWaitingForBits ), ( uxBitsToWaitFor | uxControlBits ), xTicksToWait );

            /* This is obsolete as it will get set after the task unblocks, but
            some compilers mistakenly generate a warning about the variable
            being returned without being set if it is not done. */
            uxReturn = 0;

            traceEVENT_GROUP_WAIT_BITS_BLOCK( xEventGroup, uxBitsToWaitFor );
        }
    }
    xAlreadyYielded = xTaskResumeAll();

    if( xTicksToWait != ( TickType_t ) 0 )
    {
        if( xAlreadyYielded == pdFALSE )
        {
            portYIELD_WITHIN_API();
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }

        /* The task blocked to wait for its required bits to be set - at this
        point either the required bits were set or the block time expired.  If
        the required bits were set they will have been stored in the task's
        event list item, and they should now be retrieved then cleared. */
        uxReturn = uxTaskResetEventItemValue();

        if( ( uxReturn & eventUNBLOCKED_DUE_TO_BIT_SET ) == ( EventBits_t ) 0 )
        {
            taskENTER_CRITICAL();
            {
                /* The task timed out, just return the current event bit value. */
                uxReturn = pxEventBits->uxEventBits;

                /* It is possible that the event bits were updated between this
                task leaving the Blocked state and running again. */
                if( prvTestWaitCondition( uxReturn, uxBitsToWaitFor, xWaitForAllBits ) != pdFALSE )
                {
                    if( xClearOnExit != pdFALSE )
                    {
                        pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
                    }
                    else
                    {
                        mtCOVERAGE_TEST_MARKER();
                    }
                }
                else
                {
                    mtCOVERAGE_TEST_MARKER();
                }
            }
            taskEXIT_CRITICAL();

            /* Prevent compiler warnings when trace macros are not used. */
            xTimeoutOccurred = pdFALSE;
        }
        else
        {
            /* The task unblocked because the bits were set. */
        }

        /* The task blocked so control bits may have been set. */
        uxReturn &= ~eventEVENT_BITS_CONTROL_BYTES;
    }
    traceEVENT_GROUP_WAIT_BITS_END( xEventGroup, uxBitsToWaitFor, xTimeoutOccurred );

    return uxReturn;
}
/*-----------------------------------------------------------*/

EventBits_t eventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear )
{
    EventBits_t uxReturn;

    /* Check the user is not attempting to clear the bits used by the kernel
    itself. */
    configASSERT( xEventGroup );
    configASSERT( ( uxBitsToClear & eventEVENT_BITS_CONTROL_BYTES ) == 0 );

    /* The value returned is the event group value prior to the bits being
    cleared. */
    uxReturn = ( ( EventGroup_t * ) xEventGroup )->uxEventBits;

    /* Clear the bits. */
    ( ( EventGroup_t * ) xEventGroup )->uxEventBits &= ~uxBitsToClear;

    return uxReturn;
}

EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet )
{
    ListItem_t *pxListItem, *pxNext;
    ListItem_t const *pxListEnd;
    List_t *pxList;
    EventBits_t uxBitsToClear = 0, uxBitsWaitedFor, uxControlBits;
    EventGroup_t *pxEventBits = ( EventGroup_t * ) xEventGroup;
    uint8_t xMatchFound = FALSE;

    /* Check the user is not attempting to set the bits used by the kernel itself. */
    configASSERT( xEventGroup );
    configASSERT( ( uxBitsToSet & eventEVENT_BITS_CONTROL_BYTES ) == 0 );

    pxList = &( pxEventBits->xTasksWaitingForBits );
    pxListEnd = listGET_END_MARKER( pxList ); /*lint !e826 !e740 The mini list structure is used as the list end to save RAM.  This is checked and valid. */

    pxListItem = listGET_HEAD_ENTRY( pxList );

    /* Set the bits. */
    pxEventBits->uxEventBits |= uxBitsToSet;

    /* See if the new bit value should unblock any tasks. */
    while( pxListItem != pxListEnd )
    {
        pxNext = listGET_NEXT( pxListItem );
        uxBitsWaitedFor = listGET_LIST_ITEM_VALUE( pxListItem );
        xMatchFound = FALSE;

        /* Split the bits waited for from the control bits. */
        uxControlBits = uxBitsWaitedFor & eventEVENT_BITS_CONTROL_BYTES;
        uxBitsWaitedFor &= ~eventEVENT_BITS_CONTROL_BYTES;

        if( ( uxControlBits & eventWAIT_FOR_ALL_BITS ) == ( EventBits_t ) 0 ){
            /* Just looking for single bit being set. */
            if( ( uxBitsWaitedFor & pxEventBits->uxEventBits ) != ( EventBits_t ) 0 ){
                xMatchFound = TRUE;
            }
            else{
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else if( ( uxBitsWaitedFor & pxEventBits->uxEventBits ) == uxBitsWaitedFor )
        {
            /* All bits are set. */
            xMatchFound = TRUE;
        }
        else
        {
            /* Need all bits to be set, but not all the bits were set. */
        }

        if( xMatchFound != FALSE )
        {
            /* The bits match.  Should the bits be cleared on exit? */
            if( ( uxControlBits & eventCLEAR_EVENTS_ON_EXIT_BIT ) != ( EventBits_t ) 0 ){
                uxBitsToClear |= uxBitsWaitedFor;
            }

            /* Store the actual event flag value in the task's event list
            item before removing the task from the event list.  The
            eventUNBLOCKED_DUE_TO_BIT_SET bit is set so the task knows
            that is was unblocked due to its required bits matching, rather
            than because it timed out. */
            ( void ) xTaskRemoveFromUnorderedEventList( pxListItem, pxEventBits->uxEventBits | eventUNBLOCKED_DUE_TO_BIT_SET );
        }

        /* Move onto the next list item.  Note pxListItem->pxNext is not
        used here as the list item may have been removed from the event list
        and inserted into the ready/pending reading list. */
        pxListItem = pxNext;
    }

    /* Clear any bits that matched when the eventCLEAR_EVENTS_ON_EXIT_BIT
    bit was set in the control word. */
    pxEventBits->uxEventBits &= ~uxBitsToClear;

    return pxEventBits->uxEventBits;
}

