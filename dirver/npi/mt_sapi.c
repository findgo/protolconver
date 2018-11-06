
#include "mt_sapi.h"
#include "timers.h"
#include "mleds.h"
#include "nwk.h"

#define sapi_log(format,args...)  mo_logln(DEBUG, format, ##args)

// 入网重试次数
#define ZB_START_RETRY_CNT_MAX   10

#define ZB_WAIT_RESPONSE_TIME    500    // 等待入网超时时间
#define ZB_WAIT_RESET_IND_TIME   5000   // 等待复位指示超时时间
#define ZB_WAIT_JOIN_NWK_TIME    10000  // 等待加入网络超时时间
#define ZB_RETRY_DELAY_TIME      5000   // 重试转换时间


#define ZB_STATE_IDLE                   0
#define ZB_STATE_LOGICAL_TYPE_CHECK     1
#define ZB_STATE_WRITE_LOGICAL_TYPE     2
#define ZB_STATE_START_MODEL            3
#define ZB_STATE_ON_NET                 4
#define ZB_STATE_RECOVER                5


static void mtsapi_GetDeviceAllInfohandle(uint8_t *data, uint8_t len);
static void zbStateReadLogicalType(void);
static void zbStateCheckLogicalType(uint8_t status, uint8_t type);
static void zbStateCheckWrite(uint8_t status, uint8_t type);
static void ZbtimerCb(void * arg);

nwk_info_t nwkinfo;

static TimerHandle_t ZBtimehandle;
static TimerStatic_t ZBtimerStaticBuf;

/*
int mtsapi_WriteConfiguration(uint8_t configId, uint8_t len, uint8_t *valTab)
{
    uint8_t *pbuf;
    int status;

    if((pbuf = mo_malloc( len + 2)) == NULL)
        return NPI_LNX_FAILURE;
    
    pbuf[0] = configId;
    pbuf[1] = len;

    memcpy(&pbuf[2],valTab,len);

    status = MTSAPI_SendSynchData(MT_SAPI_WRITE_CFG_REQ, pbuf, len + 2);

    mo_free(pbuf);

    return status;
}

int mtsapi_ReadConfiguration(uint8_t configId)
{
    return MTSAPI_SendSynchData(MT_SAPI_READ_CFG_REQ, &configId, 1);
}

int mtsapi_GeDeviceInfo(uint8_t Param)
{
    return MTSAPI_SendSynchData(MT_SAPI_GET_DEV_INFO_REQ, &Param, 1);
}

int mtsapi_PermitJoingRequest(uint16_t dstaddr, uint8_t timeout)
{
    uint8_t buf[3];

    buf[0] = LO_UINT16(dstaddr);
    buf[1] = HI_UINT16(dstaddr);
    buf[3] = timeout;

    return MTSAPI_SendSynchData(MT_SAPI_WRITE_CFG_REQ, buf, 3);
}
*/
/* logicaltype: ZG_DEVICETYPE_COORDINATOR,ZG_DEVICETYPE_ROUTER,ZG_DEVICETYPE_ENDDEVICE */
int mtsapi_writeLogicalType(uint8_t logicaltype)
{        
    return MTSAPI_SendSynchData(MT_SAPI_WRITE_LOGICAL_TYPE, &logicaltype, 1);
}

int mtsapi_StartNwk(uint8_t mode)
{
    return MTSAPI_SendSynchData(MT_SAPI_START_NWK, &mode, 1);
}

int mtsapi_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    switch (commandID){
    case MT_SAPI_RESET_FACTORY:
        // 指示恢复出厂设置成功,需要设备重新上电
        mledsetblink(MLED_1, 10, 50 , 1000);
        break;
        
    case MT_SAPI_GET_DEV_ALL_INFO_REQ:
        mtsapi_GetDeviceAllInfohandle(data, len);
        break;
        
    case MT_SAPI_READ_LOGICAL_TYPE:
        sapi_log("read logical");
        zbStateCheckLogicalType(data[0], data[1]);
        break;
        
    case MT_SAPI_WRITE_LOGICAL_TYPE:
        sapi_log("write logical");
        zbStateCheckWrite(data[0], data[1]);
        break;
        
    case MT_SAPI_AF_DATA_REQUEST:
        
        break;
        
    default:
        break;
    }
    
    return NPI_LNX_SUCCESS;
}

int mtsapi_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    uint8_t *msg;
    
    switch (commandID){
    case MT_SAPI_RESET_IND:
        sapi_log("reset IND");
        zbStateReadLogicalType();
        break;
    
    case MT_SAPI_AF_INCOMING_MSG:
        if((msg = msgalloc(len)) == NULL)
            return NPI_LNX_FAILURE;
        sapi_log("af incoming msg!");
        nwkmsgsend(msg); // 将消息发送nwk邮箱
        break;
                
    default:
        break;
            
    }

    return NPI_LNX_SUCCESS;
}

/****************************************************************************************************/

static void mtsapi_GetDeviceAllInfohandle(uint8_t *data, uint8_t len)
{
#if (NPI_LOGICAL_TYPE == ZG_DEVICETYPE_COORDINATOR)
    if(DEV_ZB_COORD != *data++)
#elif (NPI_LOGICAL_TYPE == ZG_DEVICETYPE_ROUTER)
    if(DEV_ROUTER != *data++)
#elif(NPI_LOGICAL_TYPE == ZG_DEVICETYPE_ENDDEVICE)
    if(DEV_END_DEVICE != *data++)
#else
    #error "Error defined device logical type!"
#endif
    {
        nwkinfo.state = ZB_STATE_IDLE;
        timerStart(ZBtimehandle, ZB_WAIT_RESPONSE_TIME); 
        return;
    }

    nwkinfo.nwkaddr = BUILD_UINT16(data[0], data[1]);
    data += 2;
    memcpy(nwkinfo.macaddr,data,Z_EXTADDR_LEN);
    data += Z_EXTADDR_LEN;
    nwkinfo.coor_nwkaddr = BUILD_UINT16(data[0], data[1]);
    data += 2;
    memcpy(nwkinfo.coor_macaddr,data,Z_EXTADDR_LEN);
    data += Z_EXTADDR_LEN;

    nwkinfo.channel = *data++;
    nwkinfo.panid = BUILD_UINT16(data[0], data[1]);
    data += 2;
    //扩展painid
    //extendpanid =  BUILD_UINT16(data[0], data[1]);

    nwkinfo.valid = TRUE;
    
    timerStop(ZBtimehandle);
    
    sapi_log("realy on net now");
}

// 状态,读设备逻辑类型
static void zbStateReadLogicalType(void)
{
    mtsapi_ReadLogicalType();
    nwkinfo.state = ZB_STATE_LOGICAL_TYPE_CHECK;
    timerStart(ZBtimehandle, ZB_WAIT_RESPONSE_TIME);
}
// 状态, 处理应答的设备逻辑类型
static void zbStateCheckLogicalType(uint8_t status, uint8_t type)
{
    // 读失败 ,几乎不可能,除非线连不对,但还是处理一下
    if(status > 0) {
        ++nwkinfo.err;
        nwkinfo.state = ZB_STATE_IDLE;
        mtsapi_SystemReset();
        timerStart(ZBtimehandle, ZB_WAIT_RESET_IND_TIME); 
        return;
    }

    // 是否是设置的类型
    if(type == NPI_LOGICAL_TYPE){
        mtsapi_StartNwk(0x02); // 启动网络
        nwkinfo.state = ZB_STATE_START_MODEL;
        timerStart(ZBtimehandle, ZB_WAIT_JOIN_NWK_TIME);
    }
    else{
        //不是设置的类型,写类型
        mtsapi_writeLogicalType(NPI_LOGICAL_TYPE);
        nwkinfo.state = ZB_STATE_WRITE_LOGICAL_TYPE;
        timerStart(ZBtimehandle, ZB_WAIT_RESPONSE_TIME);
    }
}
static void zbStateCheckWrite(uint8_t status, uint8_t type)
{
    // 写失败 ,几乎不可能,除非线连不对,但还是处理一下
    if(status > 0) {
        ++nwkinfo.err;
    }

    mtsapi_SystemReset();
    nwkinfo.state = ZB_STATE_IDLE;    
    timerStart(ZBtimehandle, ZB_WAIT_RESET_IND_TIME); 
}

void zbNwkCheckZdo(uint8_t status)
{
    if(status == 0){
        mtsapi_GeDeviceAllInfo();
        nwkinfo.state = ZB_STATE_ON_NET;
        timerStart(ZBtimehandle, ZB_WAIT_RESPONSE_TIME);
    }
    else{
        // 重试
        nwkinfo.state = ZB_STATE_RECOVER;
        timerStart(ZBtimehandle, ZB_RETRY_DELAY_TIME);
    }
}


void ZbInit()
{
    ZBtimehandle = timerAssign(&ZBtimerStaticBuf, ZbtimerCb, &ZBtimehandle);
    
    nwkinfo.state = ZB_STATE_IDLE;
    nwkinfo.valid = FALSE;
    nwkinfo.err = 0;
    mtsapi_SystemReset();
    //timerStart(ZBtimehandle, ZB_WAIT_RESET_IND_TIME);   // 等待复位指示
}

uint8_t ZbisOnNet(void)
{
    return (nwkinfo.state == ZB_STATE_ON_NET)&&( nwkinfo.valid == TRUE );
}

void zbRestart(void)
{
    nwkinfo.state = ZB_STATE_IDLE;
    nwkinfo.valid = FALSE;
    nwkinfo.err = 0;
    mtsapi_SystemReset();
    timerStart(ZBtimehandle, ZB_WAIT_RESET_IND_TIME); 
}
static void ZbtimerCb(void * arg)
{
    sapi_log("timeout");

    if(++nwkinfo.err > ZB_START_RETRY_CNT_MAX){
        nwkinfo.state = ZB_STATE_IDLE;
        // 启动入网超过次数失败,指示一下
        return;
    }
        
    switch ( nwkinfo.state){
    case ZB_STATE_IDLE:
    case ZB_STATE_LOGICAL_TYPE_CHECK:
    case ZB_STATE_WRITE_LOGICAL_TYPE:
        zbStateReadLogicalType();
        break;
        
    case ZB_STATE_START_MODEL:
        mtsapi_StartNwk(0x02);
        timerStart(ZBtimehandle, ZB_WAIT_JOIN_NWK_TIME);
        break;
        
    case ZB_STATE_ON_NET:        
        sapi_log("on net and get info!");
        mtsapi_GeDeviceAllInfo();
        timerStart(ZBtimehandle, ZB_WAIT_RESPONSE_TIME);
        break;
        
    case ZB_STATE_RECOVER:
        sapi_log("recover net!");
        mtsapi_StartNwk(0x02);
        timerStart(ZBtimehandle, ZB_WAIT_JOIN_NWK_TIME);
        break;
    }

}
