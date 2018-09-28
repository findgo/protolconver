


#include "curtain.h"
#include "wintom.h"

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
        
void curtainAttriInit(void)
{
    ltl_registerAttrList(LTL_TRUNK_ID_GENERAL_ONOFF,  1,  UBOUND(CurtainOnoffAttriList), CurtainOnoffAttriList);
    ltl_registerAttrList(LTL_TRUNK_ID_GENERAL_LEVEL_CONTROL,  1,  UBOUND(CurtainLevelControlAttriList), CurtainLevelControlAttriList);
}

void CurtainOnoff(uint8_t nodeNO, uint8_t cmd)
{
    (void) nodeNO;

    if(cmd == COMMAND_ONOFF_ON){
        curtainonoff = 1;
        curtaincurrentLevel = 255;
        //wintom_runtoPos(WT_DEV_ID_GERNERAL, curtaincurrentLevel);
    }
    else if(cmd == COMMAND_ONOFF_OFF){        
        curtainonoff = 0;
        curtaincurrentLevel = 0;
        //wintom_runtoPos(WT_DEV_ID_GERNERAL, curtaincurrentLevel);
    }
    else if(cmd == COMMAND_ONOFF_TOGGLE){
        curtainonoff = !curtainonoff;
        curtaincurrentLevel = curtainonoff ? 255 : 0;
        //wintom_runtoPos(WT_DEV_ID_GERNERAL, curtaincurrentLevel);
    }
    else {
        // ignore it

    }

    
}
void CurtainLevelControlMoveToLevel( uint8_t node, ltlLCMoveTolevel_t *pCmd )
{
    (void)node;

    curtaincurrentLevel = pCmd->level;
    if(pCmd->withOnOff){
        curtainonoff = curtaincurrentLevel ? 1 : 0;
        //wintom_runtoPos(WT_DEV_ID_GERNERAL, curtaincurrentLevel);
    }
    else{
        if(curtainonoff){
            curtainonoff = curtaincurrentLevel ? 1 : 0;
            //wintom_runtoPos(WT_DEV_ID_GERNERAL, curtaincurrentLevel);
        }
        else{
            //only change level
        }
    }
}
void CurtainLevelControlStop(uint8_t node )
{
    (void)node;

    //wintom_stop(WT_MOTO_NO_GENERAL, WT_DEV_ID_GERNERAL);
}


