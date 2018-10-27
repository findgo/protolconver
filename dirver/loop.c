
#include "loop.h"

//for driver
//#include "dlinkzigbee.h"
#include "ebyteZB.h"
#include "wintom.h"
#include "mleds.h"
#include "systick.h"
#include "nwk.h"

#include "memalloc.h"
#include "timers.h"
#include "event_groups.h"
#include "log.h"

#include "ltl.h"
#include "ltl_genattr.h"
#include "prefix.h"
#include "mt_npi.h"
#include "nv.h"

static const pTaskFn_t taskArr[] =
{
//    ebyteZBTask,
//    nwkTask,
    npiTask,
    timerTask, 
    wintomTask
};
static const uint8_t taskCnt = sizeof(taskArr) / sizeof(taskArr[0]);
static void logInit(void);

static TimerStatic_t tmstatic;
static TimerHandle_t tmhandle = NULL;
static void tmCb(void *arg);

void loop_init_System(void)
{
    Systick_Configuration();
    logInit();
    mo_logln(INFO,"loop_init_System init begin");
    
    nvinit();
    ltl_GeneralBasicAttriInit();

    delay_ms(200);
//    dl_registerParseCallBack(NULL, ltlApduParsing);
//    dlink_init();
//    nwkInit();
    npiInit();

    wintom_Init();
    halledInit();
    mledInit();

    mledset(MLED_1, MLED_MODE_FLASH);
    tmhandle = timerAssign(&tmstatic, tmCb,(void *)&tmhandle);
    timerStart(tmhandle, 1000);
    mo_logln(INFO,"loop_init_System init end, and then start system");
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
    uint16_t dst_addr = 0x0000;

    ltlReport_t *lreport;
    ltlAttrRec_t attrirecord;
    ltlReportCmd_t *reportcmd;

    if(ltlFindAttrRec(LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, ATTRID_BASIC_SERIAL_NUMBER, &attrirecord)){
 
        reportcmd = (ltlReportCmd_t *)mo_malloc(sizeof(ltlReportCmd_t) + sizeof(ltlReport_t) * 1);
        if(reportcmd){
            reportcmd->numAttr = 1;
            lreport = &(reportcmd->attrList[0]);
            lreport->attrID = attrirecord.attrId;
            lreport->dataType = attrirecord.dataType;
            lreport->attrData = attrirecord.dataPtr;

            ltl_SendReportCmd(&dst_addr, LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, 0, 
                            LTL_FRAMECTL_DIR_CLIENT_SERVER, LTL_MANU_CODE_SPECIFIC_LTL, TRUE,reportcmd);
            
            mo_free(reportcmd);
        }
    }
    timerRestart(*((TimerHandle_t *)arg), 1000);
}


static void logInit(void)
{
    SerialDrvInit(COM2, 115200, 8, DRV_PAR_NONE);
    mo_log_set_max_logger_level(LOG_LEVEL_DEBUG);
}

/* 重定向fputc 到输出，单片机一般为串口*/ 
int fputc(int ch, FILE *f)
{
    /* e.g. write a character to the USART */
    (void)Serial_WriteByte(COM2,ch);
    
//    (void)Serial_WriteByte(COM2,ch);
//    USART_SendData(USART_USING2,ch);
    /* Loop until the end of transmission */
//    while(USART_GetFlagStatus(USART_USING2, USART_FLAG_TC) == RESET);
    
    return ch;
}

