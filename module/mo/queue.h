/* 队列,操作和接收都会引起数据拷贝,*/


#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common_type.h"
#include "common_def.h"
#include "memalloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type by which queues are referenced.  For example, a call to queueNew()
 * returns an QueueHandle_t variable that can then be used as a parameter to
 * queuePut(), queuePop(), etc.
 */
typedef void * QueueHandle_t;
//静态结构体,用于屏蔽用户对结构体的可见
typedef struct QueueStatic_s
{
    void *pvDummy0[ 4 ];
    uint32_t uxDummy1[ 3 ];
} QueueStatic_t;

/* For internal use only. */
#define QUEUE_TO_BACK       ( ( uint8_t ) 0 )
#define QUEUE_TO_FRONT      ( ( uint8_t ) 1 )

QueueHandle_t queueNew( const uint32_t uxQueueItemCap, const uint32_t uxItemSize);
QueueHandle_t queueAssign( QueueStatic_t *pxStaticQueue , const uint32_t uxQueueItemCap, const uint32_t uxItemSize, uint8_t *pucQueueStorage );
uint8_t queueReset( QueueHandle_t xQueue);
void queueDelete( QueueHandle_t xQueue );

#define queuePut( xQueue, pvItemToQueue ) xQueueGenericPut( xQueue, pvItemToQueue, QUEUE_TO_BACK )
#define queuePutBack( xQueue, pvItemToQueue ) xQueueGenericPut( xQueue, pvItemToQueue, QUEUE_TO_BACK )
#define queuePutFront( xQueue, pvItemToQueue ) xQueueGenericPut( xQueue, pvItemToQueue, QUEUE_TO_FRONT )

#define queuePop( xQueue, pvBuffer) xQueueGenericPop( xQueue, pvBuffer, FALSE )
#define queuePeek( xQueue, pvBuffer) xQueueGenericPop( xQueue, pvBuffer, TRUE )

uint32_t queueItemAvailableValid(const QueueHandle_t xQueue );
uint32_t queueItemAvailableIdle( const QueueHandle_t xQueue );
uint8_t queueIsQueueEmpty( const QueueHandle_t xQueue );
uint8_t queueIsQueueFull( const QueueHandle_t xQueue );

/****************以下两个函数没有到非常了解,不得使用**************************************************************************************/
//此API并没有引起拷贝数据,它只是先占好了放数据的位置,并返回指占位的指针,均是实际位置
/* 使用方法,调用queueOnAlloc,然后直接转换,直接写数据,注意不得超过 uxItemSize 大小*/
#define queueOnAlloc( xQueue, pvItemToQueue ) xQueueOnAlloc( xQueue, QUEUE_TO_BACK )
#define queueOnAllocBack( xQueue, pvItemToQueue ) xQueueOnAlloc( xQueue, QUEUE_TO_BACK )
#define queueOnAllocFront( xQueue, pvItemToQueue ) xQueueOnAlloc( xQueue, QUEUE_TO_FRONT )
//此API并没有读回数据,而只是获取当前要出队的数据指针,这些操作均在实际数据上操作
/* 使用方法,调用queueOnPeek,然后直接转换,直接读数据,注意不要超过uxItemSize大小,使用完后,的确需要释放,调用queuePop并指定第二参数为NULL */
void *queueOnPeek( QueueHandle_t xQueue );
/******************************************************************************************************************************************/

// internal used
uint8_t xQueueGenericPut( QueueHandle_t xQueue, const void * const pvItemToQueue , const uint8_t xCopyPosition );
uint8_t xQueueGenericPop( QueueHandle_t xQueue, void * const pvBuffer, const uint8_t xJustPeeking );
void *xQueueOnAlloc( QueueHandle_t xQueue , const uint8_t xPosition );

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

