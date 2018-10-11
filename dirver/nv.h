
#ifndef __NV_H__
#define __NV_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "common_type.h"
#include "lownv.h"



/*********************************************************************
 * MACROS
 */
#define HAL_FLASH_WORD_SIZE  4

/*********************************************************************
 * FUNCTIONS
 */
#define NV_SUCCESS                  0x00
#define NV_ITEM_UNINIT              0x02
#define NV_OPER_FAILED              0x03
#define NV_BAD_ITEM_LEN             0x04
#define NV_INVALID_DATA             0x05



/*
 * Initialize NV service
 */
void nvinit( void );

/*
 * Initialize an item in NV
 */
uint8_t nvItemInit( uint16_t id, void *buf, uint16_t len );

/*
 * Read an NV attribute
 */
uint8_t nvRead( uint16_t id, uint16_t ndx, void *buf, uint16_t len );

/*
 * Write an NV attribute
 */
uint8_t nvWrite( uint16_t id, void *buf, uint16_t len );

/*
 * Get the length of an NV item.
 */
uint16_t nvItemlen( uint16_t id );

/*
 * Delete an NV item.
 */
uint8_t nvDelete( uint16_t id, uint16_t len );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OSAL_NV.H */
