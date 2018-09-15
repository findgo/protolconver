


#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common_type.h"
#include "common_def.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type by which queues are referenced.  For example, a call to queueNew()
 * returns an QueueHandle_t variable that can then be used as a parameter to
 * queueSend(), queueReceive(), etc.
 */
typedef void * QueueHandle_t;

typedef struct QueueStatic_s
{
    void *pvDummy0[ 4 ];
    uint32_t uxDummy1[ 3 ];
} QueueStatic_t;

/* For internal use only. */
#define QUEUE_SEND_TO_BACK       ( ( uint8_t ) 0 )
#define QUEUE_SEND_TO_FRONT      ( ( uint8_t ) 1 )

QueueHandle_t queueNew( const uint32_t uxQueueItemCap, const uint32_t uxItemSize);
QueueHandle_t queueAssign( QueueStatic_t *pxStaticQueue , const uint32_t uxQueueItemCap, const uint32_t uxItemSize, uint8_t *pucQueueStorage );
uint8_t queueReset( QueueHandle_t xQueue);
void queueDelete( QueueHandle_t xQueue );

#define queueSend( xQueue, pvItemToQueue ) xQueueGenericSend( xQueue, pvItemToQueue, QUEUE_SEND_TO_BACK )
#define queueSendToBack( xQueue, pvItemToQueue ) xQueueGenericSend( xQueue, pvItemToQueue, QUEUE_SEND_TO_BACK )
#define queueSendToFront( xQueue, pvItemToQueue ) xQueueGenericSend( xQueue, pvItemToQueue, QUEUE_SEND_TO_FRONT )

#define queueReceive( xQueue, pvBuffer) xQueueGenericReceive( xQueue, pvBuffer, FALSE )
#define queuePeek( xQueue, pvBuffer) xQueueGenericReceive( xQueue, pvBuffer, TRUE )

uint32_t queueItemAvailableValid(const QueueHandle_t xQueue );
uint32_t queueItemAvailableIdle( const QueueHandle_t xQueue );
uint8_t queueIsQueueEmpty( const QueueHandle_t *xQueue );
uint8_t queueIsQueueFull( const QueueHandle_t *pxQueue );

// internal used
uint8_t xQueueGenericSend( QueueHandle_t xQueue, const void * const pvItemToQueue , const uint8_t xCopyPosition );
uint8_t xQueueGenericReceive( QueueHandle_t xQueue, void * const pvBuffer, const uint8_t xJustPeeking );

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

