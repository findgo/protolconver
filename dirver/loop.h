#ifndef __LOOP_H__
#define __LOOP_H__

#include "common_type.h"
#include "common_def.h"


// 定义一个系统级别事件
#define SYS_EVENT_MSG               0x8000  // A message is waiting event

typedef uint16_t (*pTaskFn_t)( uint8_t task_id, uint16_t event );
typedef void (*pPollFn_t)(void);

#define tasks_setEventFromISR(task_id, event_flag)  tasks_setEvent(task_id, event_flag);
#define tasks_clearEventFromISR(task_id, event_flag)  tasks_clearEvent(task_id, event_flag);

uint8_t tasks_setEvent(uint8_t task_id, uint16_t event_flag);
uint8_t tasks_clearEvent(uint8_t task_id, uint16_t event_flag);
int task_msg_assign(uint8_t task_id);
int task_msg_put(uint8_t dest_taskid, void *msg_ptr);
int task_msg_putFront(uint8_t dest_taskid, void *msg_ptr);

void *tasks_msg_accept(uint8_t task_id);
void *tasks_msg_peek(uint8_t task_id);


void loop_init_System(void);
void loop_Run_System(void);

#endif

