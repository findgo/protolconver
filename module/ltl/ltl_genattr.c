



#include "ltl_genattr.h"
#include "bsp_uniqueID.h"


#define APPL_VERSION ((APPL_MAJOR_VERSION << 13) | (APPL_MINOR_VERSION << 8) \
                        | (APPL_FIXED_VERSION << 3) | APPL_BETA_VERSION)

#define HW_VERSION ((HW_MAJOR_VERSION << 13) | (HW_MINOR_VERSION << 8) \
                        | (HW_FIXED_VERSION << 3) | HW_BETA_VERSION)

//local function
static uint32_t mver_getminorver(void);
static void SerialNumber(void);
static void GenBasicResetFact(uint8_t nodeNo);
static void GenBasicReboot(void);
static void GenIdentify( uint16_t identifyTime );

//extern function
extern void CurtainOnoff(uint8_t nodeNO, uint8_t cmd);
extern void CurtainLevelControlMoveToLevel( uint8_t node, ltlLCMoveTolevel_t *pCmd );
extern void CurtainLevelControlStop(uint8_t node );


//local 
static const uint8_t ltlver = LTL_VERSION;
static const uint16_t applver = APPL_VERSION;
static const uint16_t hwver = HW_VERSION;
static char manufactTab[OCTET_CHAR_HEADROOM_LEN + MANUFACTURER_NAME_STRING_MAX_LEN];
static uint32_t buildDateCode;
static const uint32_t productID = PRODUCT_IDENTIFIER;
static uint8_t serialnumberTab[16];
static const uint8_t powersrc = POWERSOURCE_DC;


static const ltlAttrRec_t GeneralBasicAttriList[] = {
    {
        ATTRID_BASIC_LTL_VERSION,
        LTL_DATATYPE_UINT8,
        ACCESS_CONTROL_READ,
        (void *)&ltlver
    },
    {
        ATTRID_BASIC_APPL_VERSION,
        LTL_DATATYPE_UINT16,
        ACCESS_CONTROL_READ,
        (void *)&applver
    },
    {
        ATTRID_BASIC_HW_VERSION,
        LTL_DATATYPE_UINT16,
        ACCESS_CONTROL_READ,
        (void *)&hwver
    },
    {
        ATTRID_BASIC_MANUFACTURER_NAME,
        LTL_DATATYPE_CHAR_STR,
        ACCESS_CONTROL_READ,
        (void *)&manufactTab
    },
    {
        ATTRID_BASIC_BUILDDATE_CODE,
        LTL_DATATYPE_UINT32,
        ACCESS_CONTROL_READ,
        (void *)&buildDateCode
    },
    {
        ATTRID_BASIC_PRODUCT_ID,
        LTL_DATATYPE_UINT32,
        ACCESS_CONTROL_READ,
        (void *)&productID
    },
    {
        ATTRID_BASIC_SERIAL_NUMBER,
        LTL_DATATYPE_SN_ADDR,
        ACCESS_CONTROL_READ,
        (void *)&serialnumberTab
    },
    {
        ATTRID_BASIC_POWER_SOURCE,
        LTL_DATATYPE_UINT8,
        ACCESS_CONTROL_READ,
        (void *)&powersrc
    },
};
ltlGeneral_AppCallbacks_t GeneralAppCb =
{
    GenBasicResetFact,
    GenBasicReboot,
    GenIdentify,
    CurtainOnoff,
    CurtainLevelControlMoveToLevel,
    NULL,
    NULL,
    CurtainLevelControlStop,
};

void ltl_GeneralBasicAttriInit(void)
{
    ltlGeneral_RegisterCmdCallBacks(&GeneralAppCb);
    ltl_StrToAppString(MANUFACTURER_NAME, manufactTab, sizeof(manufactTab));
    buildDateCode = mver_getminorver();    
    SerialNumber();
   
    ltl_registerAttrList(LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO,
                    UBOUND(GeneralBasicAttriList), GeneralBasicAttriList);
}


static const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

/*
* __DATE__ : May 12 2017  月日年
* __TIME__ : 15:26:26     时分秒
*
*/
static uint32_t mver_getminorver(void)
{
    uint8_t i,lengh;
    uint32_t ver;
    char *pstr = __DATE__;

    lengh = strlen(pstr);
    ver = pstr[lengh - 4] - 0x30;
    ver = ver * 10 + pstr[lengh - 3] - 0x30;
    ver = ver * 10 + pstr[lengh - 2] - 0x30;
    ver = ver * 10 + pstr[lengh - 1] - 0x30;
    	
    for(i = 0;i < sizeof(months)/sizeof(months[0]); i++){
        if(memcmp(months[i],pstr,3) == 0){
            ver = ver * 100 + i + 1;
            break;
        }
    }

    lengh = 4;
    ver = ver * 10 + (pstr[lengh] == ' ' ? 0 : (pstr[lengh] - 0x30));
    ver = ver * 10 + pstr[lengh + 1] - 0x30;

    pstr = __TIME__;
    ver = ver * 10 + (pstr[0] == ' ' ? 0 : (pstr[0] - 0x30));
    ver = ver * 10 + pstr[1] - 0x30;
    
    return ver;
}

static void SerialNumber(void)
{
    mcu_info_t *info;
    
    info = bsp_GetChipInfo();
    serialnumberTab[0] = BREAK_UINT32(info->id0, 0);
    serialnumberTab[1] = BREAK_UINT32(info->id0, 1);
    serialnumberTab[2] = BREAK_UINT32(info->id0, 2);
    serialnumberTab[3] = BREAK_UINT32(info->id0, 3);

    serialnumberTab[4] = BREAK_UINT32(info->id1, 0);
    serialnumberTab[5] = BREAK_UINT32(info->id1, 1);
    serialnumberTab[6] = BREAK_UINT32(info->id1, 2);
    serialnumberTab[7] = BREAK_UINT32(info->id1, 3);
    
    serialnumberTab[8] = BREAK_UINT32(info->id2, 0);
    serialnumberTab[9] = BREAK_UINT32(info->id2, 1);
    serialnumberTab[10] = BREAK_UINT32(info->id2, 2);
    serialnumberTab[11] = BREAK_UINT32(info->id2, 3);

    serialnumberTab[12] = 0;
    serialnumberTab[13] = 0;
    serialnumberTab[14] = 0;
    serialnumberTab[15] = 0;
}


static void GenBasicResetFact(uint8_t nodeNo)
{
    (void)nodeNo;
}
static void GenBasicReboot(void)
{
    //__disable_fault_irq(); // 关中断
    //NVIC_SystemReset(); //软重启
}
static void GenIdentify( uint16_t identifyTime )
{
    (void)identifyTime;
}

