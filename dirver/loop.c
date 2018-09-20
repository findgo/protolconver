
#include "loop.h"

//for driver
#include "usart.h"
#include "dlinkzigbee.h"

#include "memalloc.h"
#include "timers.h"
#include "event_groups.h"

#include "ltl.h"
#include "ltl_genattr.h"
#include "prefix.h"


static const pTaskFn_t taskArr[] =
{
    dlinkTask,
    timerTask,   
};
static const uint8_t taskCnt = sizeof(taskArr) / sizeof(taskArr[0]);

static TimerStatic_t tmstatic;
static TimerHandle_t tmhandle = NULL;
static void tmCb(void *arg);

static TimerStatic_t tmstaticF;
static TimerHandle_t tmhandleF = NULL;
static void tmCbF(void *arg);



void loop_init_System(void)
{
    Usart_Configuration();
    ltl_GeneralAttributeInit();

    delay_ms(200);
    dl_registerParseCallBack(NULL, ltlApduParsing);
    dlink_init();

    tmhandle = timerAssign(&tmstatic, tmCb,(void *)&tmhandle);
    timerStart(tmhandle, 1000);
    tmhandleF = timerAssign(&tmstaticF, tmCbF,(void *)&tmhandleF);
    timerStart(tmhandleF, 500);
}

void loop_Run_System(void)
{
    uint8_t idx = 0;

    while(1)
    {     
        if(taskArr[idx]){
            taskArr[idx]();
        }
        ++idx;
        idx = (idx < taskCnt) ? idx : 0;
    }
}


static void tmCb(void *arg)
{
    uint16_t dst_addr = DL_BROADCAST_ADD;

    ltlReport_t *lreport;
    ltlAttrRec_t attrirecord;
    ltlReportCmd_t *reportcmd;

    if(ltlFindAttrRec(LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, ATTRID_BASIC_SERIAL_NUMBER, &attrirecord)){
 
        reportcmd = (ltlReportCmd_t *)mo_malloc(sizeof(ltlReportCmd_t) + sizeof(ltlReport_t) * 1);
        reportcmd->numAttr = 1;
        lreport = &(reportcmd->attrList[0]);
        lreport->attrID = attrirecord.attrId;
        lreport->dataType = attrirecord.dataType;
        lreport->attrData = attrirecord.dataPtr;

        ltl_SendReportCmd(&dst_addr, LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, 0, 
                        LTL_FRAMECTL_DIR_CLIENT_SERVER, LTL_MANU_CODE_SPECIFIC_LTL, TRUE,reportcmd);
        
        mo_free(reportcmd);
    }
    timerRestart(*((TimerHandle_t *)arg), 300);
}

static void tmCbF(void *arg)
{
    uint16_t dst_addr = DL_BROADCAST_ADD;

    ltlReport_t *lreport;
    ltlAttrRec_t attrirecord;
    ltlReportCmd_t *reportcmd;

    if(ltlFindAttrRec(LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, ATTRID_BASIC_MANUFACTURER_NAME, &attrirecord)){
 
        reportcmd = (ltlReportCmd_t *)mo_malloc(sizeof(ltlReportCmd_t) + sizeof(ltlReport_t) * 1);
        reportcmd->numAttr = 1;
        lreport = &(reportcmd->attrList[0]);
        lreport->attrID = attrirecord.attrId;
        lreport->dataType = attrirecord.dataType;
        lreport->attrData = attrirecord.dataPtr;

        ltl_SendReportCmd(&dst_addr, LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, 0, 
                        LTL_FRAMECTL_DIR_CLIENT_SERVER, LTL_MANU_CODE_SPECIFIC_LTL, TRUE,reportcmd);
        
        mo_free(reportcmd);
    }
    timerRestart(*((TimerHandle_t *)arg), 500);
}

