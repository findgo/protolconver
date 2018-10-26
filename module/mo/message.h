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

// 表于创建消息邮箱,表示不限消息邮箱的存储数量
#define MSGBOX_UNLIMITED_CAP    0xffff 

// 返回消息错误码
#define MSG_SUCCESS             (0)   // 成功
#define MSG_INVALID_POINTER     (-1)  // 表明指针无效
#define MSG_BUFFER_NOT_AVAIL    (-2)  // 表明释放时,消息处于队列上,不可释放
#define MSG_QBOX_FULL           (-3)  // 消息邮箱满

typedef struct {
    uint16_t dumy0; 
    uint16_t dumy1;
    void *pdumy2;
}msgboxstatic_t;
// 消息句柄
typedef void *msgboxhandle_t;
// 队列
typedef void *msg_q_t;

//静态初始化一个消息邮箱句柄缓存
#define MSGBOX_STATIC_INIT(MaxCap) { 0, (MaxCap), NULL}

// 消息
/**
 * @brief   分配一个消息
 * @param   len - 消息长度
 * @return  指向消息
 */
void *msgalloc( uint16_t len );
/**
 * @brief   释放一个消息
 * @param   msg_ptr - 指向消息
 * @return  返回错误码
 */
int msgdealloc( void *msg_ptr );
/**
 * @brief   获得消息长度
 * @param   msg_ptr - 指向消息
 * @return  返回消息长度, 指针无效返回0
 */
uint16_t msglen(void * msg_ptr);
/**
 * @brief   获得消息 预留空间的值 (供用户自定义使用)
 * @param   msg_ptr - 指向消息
 * @return  预留空间的值
 */
uint8_t msgspare(void * msg_ptr);
/**
 * @brief   设置消息 预留空间的值 (供用户自定义使用)
 * @param   msg_ptr - 指向消息
 * @return 返回错误码
 */
int msgsetspare(void * msg_ptr, uint8_t val);

//消息邮箱
/**
 * @brief   创建一个消息邮箱句柄
 * @param   MaxCap - 邮箱最大存储能力,不限制设置为 MSGBOX_UNLIMITED_CAP
 * @return 返回句柄
 */
msgboxhandle_t msgBoxNew(uint16_t MaxCap);
/**
 * @brief   用静态邮箱句柄缓存 创建 一个消息邮箱句柄
 * @param   MaxCap - 邮箱最大存储能力,不限制设置为 MSGBOX_UNLIMITED_CAP
 * @return 返回句柄
 */
msgboxhandle_t msgBoxAssign(msgboxstatic_t *pmsgboxBuffer,uint16_t MaxCap);
/**
 * @brief   获得消息邮箱中消息数量
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @return  消息数量
 */
uint16_t msgBoxcnt(msgboxhandle_t      msgbox);
/**
 * @brief   获得消息邮箱中空闲全置数量
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @return  空闲位置数量
 */
uint16_t msgBoxIdle( msgboxhandle_t msgbox );
/**
 * @brief   取出消息邮箱一条消息
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @return  有消息返回消息指针,否则为NULL
 */
void *msgBoxaccept(msgboxhandle_t msgbox);
/**
 * @brief   查看消息邮箱第一条消息,但并不取出
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @return  有消息返回消息指针,否则为NULL
 */
void *msgBoxpeek( msgboxhandle_t msgbox );
/**
 * @brief   向消息邮箱发送一条消息
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @param   msg_ptr - 消息指针
 * @return  返回错误码
 */
#define msgBoxpost(msgbox, msg_ptr)        msgBoxGenericpost(msgbox, msg_ptr, FALSE)
/**
 * @brief   向消息邮箱头 发送一条消息
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @param   msg_ptr - 消息指针
 * @return  返回错误码
 */
#define msgBoxpostFront(msgbox, msg_ptr)   msgBoxGenericpost(msgbox, msg_ptr, TRUE)

// 消息队列
// 可用于扩展消息功能时使用
/**
 * @brief   取出消息队列第一条消息
 * @param   q_ptr - 消息队列头
 * @return  有消息返回消息指针,否则为NULL
 */
void *msgQpop( msg_q_t *q_ptr );
/**
 * @brief   查看消息队列第一条消息,但不取出
 * @param   q_ptr - 消息队列头
 * @return  有消息返回消息指针,否则为NULL
 */
void *msgQpeek( msg_q_t *q_ptr );
/**
 * @brief   向消息队列 发送一条消息
 * @param   q_ptr - 消息队列头
 * @param   msg_ptr - 消息指针
 * @return  返回错误码
 */
#define msgQput( q_ptr, msg_ptr)  msgQGenericput( q_ptr, msg_ptr, FALSE )
/**
 * @brief   向消息队列头 发送一条消息
 * @param   q_ptr - 消息队列头
 * @param   msg_ptr - 消息指针
 * @return  返回错误码
 */
#define msgQputFront( q_ptr, msg_ptr)  msgQGenericput( q_ptr, msg_ptr, TRUE )
/**
 * @brief   将消息从队列中踢出 Take out of the link list
 * @param   q_ptr - 消息队列头
 * @param   msg_ptr - 消息指针
 * @param   premsg_ptr - 前一条消息指针
 * @return  返回错误码
 */
void msgQextract( msg_q_t *q_ptr, void *msg_ptr, void *premsg_ptr );

// 内部API
/**
 * @brief   向消息邮箱 发送一条消息
 * @param   msgboxhandle_t - 消息邮箱句柄
 * @param   msg_ptr - 消息指针
 * @param   isfront - FALSE: 放入消息邮箱尾 : TRUE: 放入消息邮箱头
 * @return  返回错误码
 */
int msgBoxGenericpost(msgboxhandle_t msgbox, void * msg_ptr, uint8_t isfront);
/**
 * @brief   向消息队列头 发送一条消息
 * @param   q_ptr - 消息队列头
 * @param   msg_ptr - 消息指针
 * @param   isfront - FALSE: 放入消息队列尾 : TRUE: 放入消息队列头
 * @return  返回错误码
 */
void msgQGenericput( msg_q_t *q_ptr, void *msg_ptr, uint8_t isfront );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


