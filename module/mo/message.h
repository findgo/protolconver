/*
 * 消息有两个主要API
 * msglen msgspare msgsetspare 在消息未释放前,可以使用spare 是消息里一个预留空间,8位,可供用户特殊使用
 * 
 * 消息邮箱 只可使用前面创建的消息
 *
 * 消息队列 只可使用前面创建的消息, 注意,消息队列的API,未检测指针的有效性
 *
 * NOTE: MSG_BOX_UNLIMITED_CAP 表明消息不作限制 
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

// 表于创建消息邮箱,表示不限消息邮箱的存储数量
#define MSGBOX_UNLIMITED_CAP    0xffff 

#define MSG_SUCCESS             (0)
#define MSG_INVALID_POINTER     (-1)   
#define MSG_BUFFER_NOT_AVAIL    (-2)  // 表明释放时,消息处于队列上,不可释放
#define MSG_QBOX_FULL           (-3)

typedef struct {
    uint16_t dumy0; 
    uint16_t dumy1;
    void *pdumy2;
}msgboxstatic_t;
// 消息句柄
typedef void *msgboxhandle_t;
// 队列
typedef void *msg_q_t;

// 消息
void *msgalloc( uint16_t len );
int msgdealloc( void *msg_ptr );
uint16_t msglen(void * msg_ptr);
uint8_t msgspare(void * msg_ptr);
int msgsetspare(void * msg_ptr, uint8_t val);
//消息邮箱
msgboxhandle_t msgBoxNew(uint16_t MaxCap);
msgboxhandle_t msgBoxAssign(msgboxstatic_t *pmsgboxBuffer,uint16_t MaxCap);
uint16_t msgBoxcnt(msgboxhandle_t      msgbox);
uint16_t msgBoxIdle( msgboxhandle_t msgbox );
void *msgBoxaccept(msgboxhandle_t msgbox);
void *msgBoxpeek( msgboxhandle_t msgbox );
#define msgBoxpost(msgbox, msg_ptr)        msgBoxGenericpost(msgbox, msg_ptr, FALSE)
#define msgBoxpostFront(msgbox, msg_ptr)   msgBoxGenericpost(msgbox, msg_ptr, TRUE)

// 消息队列
void *msgQpop( msg_q_t *q_ptr );
void *msgQpeek( msg_q_t *q_ptr );
#define msgQput( q_ptr, msg_ptr)  msgQGenericput( q_ptr, msg_ptr, FALSE )
#define msgQputFront( q_ptr, msg_ptr)  msgQGenericput( q_ptr, msg_ptr, TRUE )
// Take out of the link list
void msgQextract( msg_q_t *q_ptr, void *msg_ptr, void *premsg_ptr );

// 内部API
int msgBoxGenericpost(msgboxhandle_t msgbox, void * msg_ptr, uint8_t isfront);
void msgQGenericput( msg_q_t *q_ptr, void *msg_ptr, uint8_t isfront );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


