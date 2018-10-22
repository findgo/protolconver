/*
 * 消息的用法
 * 用法1: 采用自带方法, msg_allocate 得到一个消息实例, 使用完毕采用msg_deallocate 释放, 用msg_send或msg_send_front发送,并可以带唯一ID进行识别
 *        用msg_receive进行接收,当ID为MSG_ID_NO_USED 表示按顺序接收. 否则只查找唯一ID的消息. 
 *        以上不得调用msg_setid和msg_setlen API
 *
 * 用法2: 自定义队列msg_q_t testq, msg_allocate 得到一个消息实例, 使用完毕采用msg_deallocate 释放, 可以msg_setid,msg_setlen设置消息ID或消息长度.
 *          msg_queueput消息入队,msg_queuepop消息出队
 *         note: 这里的id已经没什么用了,用户可以自己定义用图
 */

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

// 用于msg_receive 表示按顺序取出消息
#define MSG_ID_NO_USED  0xffff

#define MSG_SUCCESS             0x00
#define MSG_INVALID_POINTER     0x01   
#define MSG_BUFFER_NOT_AVAIL    0x02
#define MSG_INVALID_ID          0x03

typedef void *msg_q_t;

uint8_t *msg_allocate( uint16_t len );
uint8_t msg_deallocate( void *msg_ptr );
uint8_t msg_send( uint16_t id, uint8_t *msg_ptr );
uint8_t msg_send_front( uint16_t id, uint8_t *msg_ptr );
uint8_t *msg_receive( uint16_t id );
uint16_t msg_id(uint8_t * msg_ptr);
uint16_t msg_len(uint8_t * msg_ptr);

#define msg_queuecput(q_ptr, msg_ptr)       msg_queueGenericput(q_ptr, msg_ptr, FALSE)
#define msg_queuecput_front(q_ptr, msg_ptr) msg_queueGenericput(q_ptr, msg_ptr, TRUE)
void *msg_queuepop( msg_q_t *q_ptr );
// 以下方法仅在自定义消息队列时使用,也就是方法2
uint8_t msg_setid(uint8_t * msg_ptr, uint16_t id);
uint8_t msg_setlen(uint8_t * msg_ptr, uint16_t len);

// 内部API
void msg_queueGenericput( msg_q_t *q_ptr, void *msg_ptr, uint8_t isfront );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


