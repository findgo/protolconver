


#include "curtain.h"

#include "mleds.h"
#include "timers.h"

#include "wintom.h"

// 周期 1s 读取位置
#define CURTAIN_POS_SCAN_TIME  1000

static uint8_t curtainonoff;
static uint8_t curtaincurrentLevel;


static const ltlAttrRec_t CurtainOnoffAttriList[] = {
    {
        ATTRID_ONOFF_STATUS,
        LTL_DATATYPE_BOOLEAN,
        ACCESS_CONTROL_READ,
        (void *)&curtainonoff
    },
};
    

static const ltlAttrRec_t CurtainLevelControlAttriList[] = {
    {
        ATTRID_LEVEL_CURRENT_LEVEL,
        LTL_DATATYPE_UINT8,
        ACCESS_CONTROL_READ,
        (void *)&curtaincurrentLevel
    },
};
        
static timer_t timerCurtain;

static void tmCurtainCb(void *arg);
static void curtainParsePoscb(uint8_t moto_no, uint8_t devID, uint8_t pos );

static wintom_rspCallbacks_t wintom_rspcb = {curtainParsePoscb, NULL};

void curtainAttriInit(void)
{
    ltl_registerAttrList(LTL_TRUNK_ID_GENERAL_ONOFF,  LTL_NODE_CURTAIN,  UBOUND(CurtainOnoffAttriList), CurtainOnoffAttriList);
    ltl_registerAttrList(LTL_TRUNK_ID_GENERAL_LEVEL_CONTROL,  LTL_NODE_CURTAIN,  UBOUND(CurtainLevelControlAttriList), CurtainLevelControlAttriList);    
    wintom_registerRspCallBack(&wintom_rspcb);
    
    timerAssign(&timerCurtain, tmCurtainCb, (void *)&timerCurtain);
    timerStart(&timerCurtain, CURTAIN_POS_SCAN_TIME);
}

void CurtainOnoff(uint8_t nodeNO, uint8_t cmd)
{
    (void) nodeNO;

    if(cmd == COMMAND_ONOFF_ON){
        curtainonoff = 1;
        wintom_open(WT_MOTO_NO_GENERAL, WT_DEV_ID_GERNERAL);
    }
    else if(cmd == COMMAND_ONOFF_OFF){        
        curtainonoff = 0;
        wintom_close(WT_MOTO_NO_GENERAL, WT_DEV_ID_GERNERAL);
    }
    else if(cmd == COMMAND_ONOFF_TOGGLE){
        curtainonoff = !curtainonoff;
        curtainonoff ? wintom_open(WT_MOTO_NO_GENERAL, WT_DEV_ID_GERNERAL) : wintom_close(WT_MOTO_NO_GENERAL, WT_DEV_ID_GERNERAL);
    }
    else {
        // ignore it

    }

    
}
void CurtainLevelControlMoveToLevel( uint8_t node, ltlLCMoveTolevel_t *pCmd )
{
    (void)node;

    if(pCmd->withOnOff){
        curtainonoff = curtaincurrentLevel ? 1 : 0;
        wintom_runtoPos(WT_DEV_ID_GERNERAL, pCmd->level);
    }
    else{
        if(curtainonoff){
            curtainonoff = curtaincurrentLevel ? 1 : 0;
            wintom_runtoPos(WT_DEV_ID_GERNERAL, pCmd->level);
        }
        else {
            //only change level
        }
    }
}
void CurtainLevelControlStop(uint8_t node )
{
    (void)node;

    wintom_stop(WT_MOTO_NO_GENERAL, WT_DEV_ID_GERNERAL);
}

static void tmCurtainCb(void *arg)
{
    wintom_getPos(WT_DEV_ID_GERNERAL);
    timerRestart((timer_t *)arg, CURTAIN_POS_SCAN_TIME);
}


static void curtainParsePoscb(uint8_t moto_no, uint8_t devID, uint8_t pos )
{
    uint16_t dst_addr = 0x0000;

    ltlReport_t *lreport;
    const ltlAttrRec_t *attrirecord;
    ltlReportCmd_t *reportcmd;
    (void)moto_no;
    (void)devID;

    if(curtaincurrentLevel != pos){
        curtaincurrentLevel = pos;
        // report level and on off
        reportcmd = (ltlReportCmd_t *)mo_malloc(sizeof(ltlReportCmd_t) + sizeof(ltlReport_t) * 2);
        reportcmd->numAttr = 2;
        lreport = &(reportcmd->attrList[0]);            
        attrirecord = &CurtainOnoffAttriList[0];
        lreport->attrID = attrirecord->attrId;
        lreport->dataType = attrirecord->dataType;
        lreport->attrData = attrirecord->dataPtr;
        
        lreport = &(reportcmd->attrList[1]);         
        attrirecord = &CurtainLevelControlAttriList[0];
        lreport->attrID = attrirecord->attrId;
        lreport->dataType = attrirecord->dataType;
        lreport->attrData = attrirecord->dataPtr;

        
        ltl_SendReportCmd(dst_addr, LTL_TRUNK_ID_GENERAL_LEVEL_CONTROL, LTL_NODE_CURTAIN, 0, 
                            TRUE, reportcmd);
        
        mo_free(reportcmd);
    }
}
