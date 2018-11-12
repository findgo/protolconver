

#ifndef __MO_TASK_H__
#define __MO_TASK_H__

#include "common_type.h"
#include "porting_def.h"
#include "msglink.h"
#include "port_atom.h"

// 定义一个系统级别事件
#define SYS_EVENT_MSG               0x8000  // A message is waiting event

typedef uint16_t (*pTaskFn_t)( uint8_t task_id, uint16_t event );
typedef void (*pPollFn_t)(void);

#if configSUPPORT_POLL_TASK > 0
extern const pPollFn_t pollsArr[];
extern const uint8_t poolsCnt;
#endif

#if configSUPPORT_TASKS_EVENT > 0
extern const pTaskFn_t tasksArr[];
extern const uint8_t tasksCnt;
extern uint16_t *tasksEvents;
#if defined(configSUPPORT_POWER_SAVING)
extern void powerconserve(void);
#endif
#endif

#define tasks_setEventFromISR(task_id, event_flag)  tasks_setEvent(task_id, event_flag);
#define tasks_clearEventFromISR(task_id, event_flag)  tasks_clearEvent(task_id, event_flag);

uint8_t tasks_setEvent(uint8_t task_id, uint16_t event_flag);
uint8_t tasks_clearEvent(uint8_t task_id, uint16_t event_flag);
int task_msg_assign(uint8_t task_id);
int task_msg_put(uint8_t dest_taskid, void *msg_ptr);
int task_msg_putFront(uint8_t dest_taskid, void *msg_ptr);

void *tasks_msg_accept(uint8_t task_id);
void *tasks_msg_peek(uint8_t task_id);
void tasks_Run_System(void);


#endif

