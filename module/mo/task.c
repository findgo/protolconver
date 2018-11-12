#include "task.h"

void tasks_Run_System(void)
{
    uint8_t idx = 0;
    uint16_t events;
    
#if configSUPPORT_POLL_TASK > 0
    for(idx = 0; idx < poolsCnt; ++idx){
        pollsArr[idx]();
    }
#endif

#if configSUPPORT_TASKS_EVENT > 0
    // 移植的TI OSAL
    for(idx = 0;idx < tasksCnt; ++idx)
    {
        if (tasksEvents[idx]){ // Task is highest priority that is ready.
            break;
        }
    }
    
    if (idx < tasksCnt) {
    
        taskENTER_CRITICAL();
        events = tasksEvents[idx];
        tasksEvents[idx] = 0;  // Clear the Events for this task.
        taskEXIT_CRITICAL();
    
        events = (tasksArr[idx])( idx, events );
    
        taskENTER_CRITICAL();
        tasksEvents[idx] |= events;  // Add back unprocessed events to the current task.
        taskEXIT_CRITICAL();
    }
#if defined( configSUPPORT_POWER_SAVING )
    else { // Complete pass through all task events with no activity?
        powerconserve();  // Put the processor/system into sleep
    }
#endif
#endif

}

#if configSUPPORT_TASKS_EVENT > 0
// task id and qmsg map message
typedef struct {
    uint8_t task_id;
    msg_q_t q_taskmsghead;
}taskmsg_mapInfo_t; 
static taskmsg_mapInfo_t *taskmap_search(uint8_t task_id);

static msg_q_t taskmsgq_head;

uint8_t tasks_setEvent(uint8_t task_id, uint16_t event_flag)
{
    isrSaveCriticial_status_Variable;

    if ( task_id < tasksCnt ){
        isrENTER_CRITICAL();
        tasksEvents[task_id] |= event_flag; 
        isrEXIT_CRITICAL();
        return TRUE;
    }
    
    return FALSE;
}

uint8_t tasks_clearEvent(uint8_t task_id, uint16_t event_flag)
{
    isrSaveCriticial_status_Variable;

    if ( task_id < tasksCnt ){
        isrENTER_CRITICAL();
        tasksEvents[task_id] &= ~event_flag; 
        isrEXIT_CRITICAL();
        return TRUE;
    }
    
    return FALSE;
}

int task_msg_assign(uint8_t task_id)
{
    taskmsg_mapInfo_t *mapinfo;

    mapinfo = taskmap_search(task_id);

    if(mapinfo == NULL){
        if ((mapinfo = msgalloc(sizeof(taskmsg_mapInfo_t))) == NULL)
            return MSG_INVALID_POINTER;

        mapinfo->task_id = task_id;
        mapinfo->q_taskmsghead = NULL;

        msgQputFront(&taskmsgq_head, mapinfo);
    }
    
    return MSG_SUCCESS;
}

// not find it : NULL,else find it 
static taskmsg_mapInfo_t *taskmap_search(uint8_t task_id)
{
    taskmsg_mapInfo_t *srch;

    msgQ_for_each_msg(&taskmsgq_head, srch){
        if(srch->task_id == task_id){ // find it 
            break;
        }
    }

    return srch;
}

int task_msg_Genericput(uint8_t dest_taskid, void *msg_ptr, uint8_t isfront)
{
    taskmsg_mapInfo_t *mapinfo;

    if(dest_taskid >= tasksCnt){
        return MSG_INVALID_TASK;
    }

    mapinfo = taskmap_search(dest_taskid);
    if(!mapinfo){
        return MSG_TASK_MSG_UNINT;
    }

    msgQGenericput(&(mapinfo->q_taskmsghead), msg_ptr, isfront);
    tasks_setEvent(dest_taskid, SYS_EVENT_MSG);
    
    return MSG_SUCCESS;
}
int task_msg_put(uint8_t dest_taskid, void *msg_ptr)
{
    return task_msg_Genericput(dest_taskid, msg_ptr, FALSE);
}
int task_msg_putFront(uint8_t dest_taskid, void *msg_ptr)
{
    return task_msg_Genericput(dest_taskid, msg_ptr, TRUE);
}
void *tasks_msg_accept(uint8_t task_id)
{
    taskmsg_mapInfo_t *mapinfo;

    if(task_id >= tasksCnt){
        return NULL;
    }

    mapinfo = taskmap_search(task_id);
    if(!mapinfo){
        return NULL;
    }

    return msgQpop(&(mapinfo->q_taskmsghead));
}

void *tasks_msg_peek(uint8_t task_id)
{
    taskmsg_mapInfo_t *mapinfo;

    if(task_id >= tasksCnt){
        return NULL;
    }

    mapinfo = taskmap_search(task_id);
    if(!mapinfo){
        return NULL;
    }

    return msgQpeek(&(mapinfo->q_taskmsghead));
}

#endif

