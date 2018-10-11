

/* ID 0x0000 和0xffff保留 */
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
 * 确定NV总线的电压
 */
# define  NV_CHECK_BUS_VOLTAGE   1

/*********************************************************************
 * MACROS
 */
#define HAL_FLASH_WORD_SIZE  4

/*********************************************************************
 * FUNCTIONS
 */
#define NV_SUCCESS                  0x00
#define NV_ITEM_UNINIT              0x01
#define NV_OPER_FAILED              0x02



/* Initialize NV service */
void nvinit( void );

/* Read an NV attribute */
uint8_t nvItemReadx( uint16_t id, uint16_t ndx, void *buf, uint16_t len );

/* Read an NV attribute */
#define nvItemRead(id, buf, len)    nvItemReadx(id, 0, buf, len)

/* Write an NV attribute*/
uint8_t nvItemWrite( uint16_t id, void *buf, uint16_t len );

/* Get the length of an NV item. */
uint16_t nvItemlen( uint16_t id );

/* Delete an NV item. */
uint8_t nvItemDelete( uint16_t id );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OSAL_NV.H */
