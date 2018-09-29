

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "common_type.h"
#include "common_def.h"
#include "memalloc.h"

#ifdef __cplusplus
extern "C" {
#endif
// support msg?
#define configSUPPORT_MSG       1


#define MSG_ID_NO_USED  0xffff

#define MSG_SUCCESS             0x00
#define MSG_INVALID_POINTER     0x01   
#define MSG_BUFFER_NOT_AVAIL    0x02
#define MSG_INVALID_ID          0x03

uint8_t *msg_allocate( uint16_t len );
uint8_t msg_deallocate( void *msg_ptr );
uint8_t msg_send( uint16_t id, uint8_t *msg_ptr );
uint8_t msg_send_front( uint16_t id, uint8_t *msg_ptr );
uint8_t *msg_receive( uint16_t id );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


