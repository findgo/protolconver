

#include "ltl_general.h"


// local function
static LStatus_t ltlGeneral_SpecificTrunckhandle( ltlApduMsg_t *ApduMsg );
static LStatus_t ltlGeneral_ProcessSpecificInbasic( ltlApduMsg_t *ApduMsg );
static LStatus_t ltlGeneral_ProcessSpecificInonoff( ltlApduMsg_t *ApduMsg );
// local variable
static uint8_t ltlGeneralRegisted = FALSE;
static ltlGeneral_AppCallbacks_t *ltlGeneralCBs = NULL;

/*********************************************************************
 * @brief   注册通用集下所有集命令解析回调
 *
 * @param   callbacks - in --- callback list
 *
 * @return 
 */
LStatus_t ltlGeneral_RegisterCmdCallBacks(ltlGeneral_AppCallbacks_t *callbacks)
{
    if(ltlGeneralRegisted == FALSE){
        
        ltl_registerPlugin(LTL_TRUNK_ID_GENERAL_BASIC, LTL_TRUNK_ID_GENERAL_ONOFF, ltlGeneral_SpecificTrunckhandle);
        ltlGeneralCBs = callbacks;

        ltlGeneralRegisted = TRUE;
    }

    return LTL_SUCCESS;
}


/*********************************************************************
 * @brief   通用集下所有集命令解析的回调
 *
 * @param   ApduMsg - in --- apdu message
 *
 * @return 
 */
static LStatus_t ltlGeneral_SpecificTrunckhandle( ltlApduMsg_t *ApduMsg )
{
    LStatus_t status = LTL_STATUS_SUCCESS;

    if( ltl_IsTrunkCmd(ApduMsg->hdr.fc.type) ){
        if( ltl_IsMancodeDisable(ApduMsg->hdr.fc.manuSpecific) ){
            if(ltlGeneralCBs == NULL)
                return LTL_STATUS_FAILURE;
   // gerneral specific trunk process add here         
            switch (ApduMsg->hdr.trunkID){
            case LTL_TRUNK_ID_GENERAL_BASIC:
                status = ltlGeneral_ProcessSpecificInbasic(ApduMsg);
                break;
            case LTL_TRUNK_ID_GENERAL_ONOFF:
                status = ltlGeneral_ProcessSpecificInonoff(ApduMsg);
                break;



                 
             default:
                status = LTL_STATUS_FAILURE;
                break;
            }           
        }else{
            //TODO: later
            status = LTL_STATUS_FAILURE;
        }
    }
    else{
        status = LTL_STATUS_FAILURE;
        //should not go here

    }

    return(status);
}
/*********************************************************************
 * @brief   通用集下 基本集命令解析回调
 *
 * @param   ApduMsg - in --- apdu message
 *
 * @return 
 */
static LStatus_t ltlGeneral_ProcessSpecificInbasic( ltlApduMsg_t *ApduMsg )
{
    uint32_t tmpvalue;

    if(ltl_ServerCmd(ApduMsg->hdr.fc.direction)){
        
        switch( ApduMsg->hdr.commandID ){
        case COMMAND_BASIC_RESET_FACT_DEFAULT:
            if(ltlGeneralCBs->pfnBasicResetFact)
                ltlGeneralCBs->pfnBasicResetFact(ApduMsg->hdr.nodeNo);
            break;
            
        case COMMAND_BASIC_REBOOT_DEVICE:
            if(ltlGeneralCBs->pfnBasicReboot)
                ltlGeneralCBs->pfnBasicReboot();
            break;
            
        case COMMAND_BASIC_IDENTIFY:
            tmpvalue = BUILD_UINT16(ApduMsg->pdata[0], ApduMsg->pdata[1]);
            if(ltlGeneralCBs->pfnIdentify)
                ltlGeneralCBs->pfnIdentify((uint16_t)tmpvalue);
            break;
            
        default:
            return LTL_STATUS_FAILURE;
            //break;
        }
    }

    return LTL_STATUS_SUCCESS;
}
/*********************************************************************
 * @brief   通用集下 onoff命令解析回调
 *
 * @param   ApduMsg - in --- apdu message
 *
 * @return 
 */
static LStatus_t ltlGeneral_ProcessSpecificInonoff( ltlApduMsg_t *ApduMsg )
{
    if(ltl_ServerCmd(ApduMsg->hdr.fc.direction)){
        if(ApduMsg->hdr.commandID > COMMAND_ONOFF_TOGGLE){
            return LTL_STATUS_FAILURE;
        }
        else{
            if(ltlGeneralCBs->pfnOnoff)
                ltlGeneralCBs->pfnOnoff(ApduMsg->hdr.commandID);
        }
    }

    return LTL_STATUS_SUCCESS;
}

