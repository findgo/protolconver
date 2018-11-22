#ifndef __LTL_H_
#define __LTL_H_

#include "common_type.h"
#include "common_def.h"
#include "prefix.h"

// define in frame control field
//bit mask
#define LTL_FRAMECTL_TYPE_MASK             0x03
#define LTL_FRAMECTL_DIR_MASK              0x04
#define LTL_FRAMECTL_DISALBE_DEFAULT_RSP_MASK  0x08
//subfield type
#define LTL_FRAMECTL_TYPE_PROFILE          0x00
#define LTL_FRAMECTL_TYPE_TRUNK_SPECIFIC   0x01
//subfield Manufacturer code
#define LTL_FRAMECTL_DIR_CLIENT_SERVER 0
#define LTL_FRAMECTL_DIR_SERVER_CLIENT 1
//subfield disable default response
#define LTL_FRAMECTL_DIS_DEFAULT_RSP_OFF 0
#define LTL_FRAMECTL_DIS_DEFAULT_RSP_ON 1

// General command IDs on profile 
#define LTL_CMD_READ_ATTRIBUTES                 0x00
#define LTL_CMD_READ_ATTRIBUTES_RSP             0x01
#define LTL_CMD_WRITE_ATTRIBUTES                0x02
#define LTL_CMD_WRITE_ATTRIBUTES_UNDIVIDED      0x03
#define LTL_CMD_WRITE_ATTRIBUTES_RSP            0x04
#define LTL_CMD_WRITE_ATTRIBUTES_NORSP          0x05
#define LTL_CMD_CONFIGURE_REPORTING             0x06 // 读写 报告配置未实现
#define LTL_CMD_CONFIGURE_REPORTING_RSP         0x07
#define LTL_CMD_READ_CONFIGURE_REPORTING        0x08
#define LTL_CMD_READ_CONFIGURE_REPORTING_RSP    0x09
#define LTL_CMD_REPORT_ATTRIBUTES               0x0a
#define LTL_CMD_DEFAULT_RSP                     0x0b
#define LTL_CMD_READ_ATTRIBUTES_STRUCTURED      0x0c // 结构型未实现
#define LTL_CMD_READ_ATTRIBUTES_STRUCTURED_RSP  0x0d
#define LTL_CMD_WRITE_ATTRIBUTES_STRUCTURED     0x0e
#define LTL_CMD_WRITE_ATTRIBUTES_STRUCTURED_RSP 0x0f
#define LTL_CMD_PROFILE_MAX                     LTL_CMD_DEFAULT_RSP

/*** Data Types (32) ***/
#define LTL_DATATYPE_NO_DATA                            0x00
#define LTL_DATATYPE_DATA8                              0x05
#define LTL_DATATYPE_DATA16                             0x06
#define LTL_DATATYPE_DATA32                             0x07
#define LTL_DATATYPE_DATA64                             0x08
#define LTL_DATATYPE_BOOLEAN                            0x10
#define LTL_DATATYPE_BITMAP8                            0x15
#define LTL_DATATYPE_BITMAP16                           0x16
#define LTL_DATATYPE_BITMAP32                           0x17
#define LTL_DATATYPE_BITMAP64                           0x18
#define LTL_DATATYPE_UINT8                              0x25
#define LTL_DATATYPE_UINT16                             0x26
#define LTL_DATATYPE_UINT32                             0x27
#define LTL_DATATYPE_UINT64                             0x28
#define LTL_DATATYPE_INT8                               0x35
#define LTL_DATATYPE_INT16                              0x36
#define LTL_DATATYPE_INT32                              0x37
#define LTL_DATATYPE_INT64                              0x38
#define LTL_DATATYPE_ENUM8                              0x3a
#define LTL_DATATYPE_ENUM16                             0x3b
#define LTL_DATATYPE_SINGLE_PREC                        0x3c
#define LTL_DATATYPE_DOUBLE_PREC                        0x3d
#define LTL_DATATYPE_OCTET_ARRAY                        0x41
#define LTL_DATATYPE_CHAR_STR                           0x42
#define LTL_DATATYPE_LONG_OCTET_ARRAY                   0x43
#define LTL_DATATYPE_LONG_CHAR_STR                      0x44
#define LTL_DATATYPE_ARRAY                              0x51
#define LTL_DATATYPE_TRUNK_ID                           0x52
#define LTL_DATATYPE_ATTR_ID                            0x53
#define LTL_DATATYPE_SN_ADDR                            0x54
#define LTL_DATATYPE_128_BIT_SEC_KEY                    0x55
#define LTL_DATATYPE_UNKNOWN                            0xff

/*** Error Status Codes ***/
#define LTL_STATUS_SUCCESS                              0x00 //操作成功
#define LTL_STATUS_FAILURE                              0x01 //操作失败
// 0x02-0x7D are reserved.
#define LTL_STATUS_NOT_AUTHORIZED                       0x7E // 未授权
#define LTL_STATUS_MALFORMED_COMMAND                    0x80
#define LTL_STATUS_UNSUP_TRUNK_COMMAND                  0x81  //不支持集下命令
#define LTL_STATUS_UNSUP_GENERAL_COMMAND                0x82   //不支持profile下的通用标准命令
//#define LTL_STATUS_UNSUP_MANU_TRUNK_COMMAND             0x83  // 不支持集下的制造商命令
//#define LTL_STATUS_UNSUP_MANU_GENERAL_COMMAND           0x84  // 不支持profile下制造商的命令
#define LTL_STATUS_INVALID_FIELD                        0x85  // 域无效,一般表现为发送的值域对设备无影响
#define LTL_STATUS_UNSUPPORTED_ATTRIBUTE                0x86  //不支持的属性
#define LTL_STATUS_INVALID_VALUE                        0x87  // 无效数值
#define LTL_STATUS_READ_ONLY                            0x88   // 只读
#define LTL_STATUS_NOT_FOUND                            0x8a  //请求的信息没有找到
#define LTL_STATUS_UNREPORTABLE_ATTRIBUTE               0x8b  // 这个属性不能定期报告
#define LTL_STATUS_INVALID_DATA_TYPE                    0x8c    //无效数据类型
#define LTL_STATUS_WRITE_ONLY                           0x8d   //只写
#define LTL_STATUS_DEFINED_OUT_OF_BAND                  0x8e   // 写的数据超过范围
#define LTL_STATUS_INCONSISTENT                         0x8f
#define LTL_STATUS_ACTION_DENIED                        0x90  // 拒绝此命令动作
#define LTL_STATUS_TIMEOUT                              0x91  //超时
#define LTL_STATUS_ABORT                                0x92  //停止
//#define LTL_STATUS_INVALID_IMAGE                        0x96  //无效镜像
//#define LTL_STATUS_WAIT_FOR_DATA                        0x97  //等待数据
//#define LTL_STATUS_NO_IMAGE_AVAILABLE                   0x98  // 没有有效的镜像
//#define LTL_STATUS_REQUIRE_MORE_IMAGE                   0x99  //请求更多的镜像
#define LTL_STATUS_HARDWARE_FAILURE                     0xc0  // 硬件问题错误
#define LTL_STATUS_SOFTWARE_FAILURE                     0xc1  // 软件错误
// 0xc3-0xff are reserved.
#define LTL_STATUS_CMD_HAS_RSP                          0xff

/*** Attribute Access Control - bit masks ***/
#define ACCESS_CONTROL_READ                             0x01
#define ACCESS_CONTROL_WRITE                            0x02
#define ACCESS_CONTROL_COMMAND                          0x04
#define ACCESS_CONTROL_AUTH_READ                        0x10
#define ACCESS_CONTROL_AUTH_WRITE                       0x20

// Used by ltlReadWriteCB_t callback function
#define LTL_OPER_LEN                                    0x00 // Get length of attribute value to be read
#define LTL_OPER_READ                                   0x01 // Read attribute value
#define LTL_OPER_WRITE                                  0x02 // Write new attribute value

#define LTL_SUCCESS  0x00
#define LTL_FAILURE  0x01  
#define LTL_MEMERROR 0x02



// 对于应用层的数据类型的一些 字节串 字符串 数组进行定义预留长度
#define OCTET_CHAR_HEADROOM_LEN         (1) // length : 1
#define OCTET_CHAR_LONG_HEADROOM_LEN    (2) // length : 2
#define ARRAY_HEADROOM_LEN              (2) // element type + number of element : 1 +　1


/*********************************************************************
 * MACROS
 */
#define ltl_IsProfileCmd( a )           ( (a) == LTL_FRAMECTL_TYPE_PROFILE )
#define ltl_IsTrunkCmd( a )             ( (a) == LTL_FRAMECTL_TYPE_TRUNK_SPECIFIC )

// ltl_ServerCmd client to server
// ltl_ClientCmd server to client
#define ltl_ServerCmd( a )       ( (a) == LTL_FRAMECTL_DIR_CLIENT_SERVER )
#define ltl_ClientCmd( a )       ( (a) == LTL_FRAMECTL_DIR_SERVER_CLIENT )

typedef uint8_t LStatus_t;

// LTL header - frame control field
typedef struct
{
    uint8_t type;
    uint8_t direction;
    uint8_t disableDefaultRsp;
    uint8_t reserved;
} ltlFrameHdrctl_t;

// LTL header
typedef struct
{
    uint16_t    trunkID;
    uint8_t     transSeqNum;
    uint8_t     nodeNo;
    uint8_t     commandID;
    ltlFrameHdrctl_t fc;
} ltlFrameHdr_t;

//解析完包头的APDU包
typedef struct 
{
    MoIncomingMsgPkt_t *pkt;
    ltlFrameHdr_t hdr;  /* adpu head frame  */
    uint16_t datalen; // length of remaining data
    uint8_t *pdata;  // point to data after header
    void *attrCmd; // point to the parsed attribute or command
}ltlApduMsg_t;

// Parse received command
typedef struct
{
    uint16_t trunkID;
    uint8_t  nodeNO;
    uint16_t dataLen;
    uint8_t  *pData;
} ltlParseCmd_t;

/*------------------------for application-----------------------------------------------*/

// Attribute record
typedef struct
{
  uint16_t  attrId;         // Attribute ID
  uint8_t   dataType;       // Data Type 
  uint8_t   accessControl;  // Read/write acess - bit field
  void    *dataPtr;       // Pointer to data field
} ltlAttrRec_t;



// for read
// Read Attribute Command format
// be allocated with the appropriate number of attributes.
typedef struct
{
  uint8_t  numAttr;            // number of attributes in the list
  uint16_t attrID[];           // supported attributes list - this structure should
} ltlReadCmd_t;

// Read Attribute Response Status record format
typedef struct
{
  uint16_t attrID;            // attribute ID
  uint8_t  status;            // should be ZCL_STATUS_SUCCESS or error
  uint8_t  dataType;          // attribute data type
  uint8_t  *data;             // this structure is allocated, so the data is HERE
                            // - the size depends on the attribute data type
} ltlReadRspStatus_t;

// Read Attribute Response Command format
typedef struct
{
  uint8_t            numAttr;     // number of attributes in the list
  ltlReadRspStatus_t attrList[];  // attribute status list
} ltlReadRspCmd_t;

// for write
// Write Attribute record
typedef struct
{
  uint16_t attrID;             // attribute ID
  uint8_t  dataType;           // attribute data type
  uint8_t  *attrData;          // this structure is allocated, so the data is HERE
                             //  - the size depends on the attribute data type
} ltlWriteRec_t;

// Write Attribute Command format
typedef struct
{
  uint8_t         numAttr;     // number of attribute records in the list
  ltlWriteRec_t attrList[];  // attribute records
} ltlWriteCmd_t;
// Write Attribute Status record
typedef struct
{
  uint8_t  status;             // should be LTL_STATUS_SUCCESS or error
  uint16_t attrID;             // attribute ID
} ltlWriteRspStatus_t;

// Write Attribute Response Command format
typedef struct
{
  uint8_t               numAttr;     // number of attribute status in the list
  ltlWriteRspStatus_t attrList[];  // attribute status records
} ltlWriteRspCmd_t;

// Attribute Report
typedef struct
{
  uint16_t attrID;             // atrribute ID
  uint8_t  dataType;           // attribute data type
  uint8_t  *attrData;          // this structure is allocated, so the data is HERE
                             // - the size depends on the data type of attrID
} ltlReport_t;

// Report Attributes Command format
typedef struct
{
  uint8_t       numAttr;       // number of reports in the list
  ltlReport_t attrList[];    // attribute report list
} ltlReportCmd_t;


// Default Response Command format
typedef struct
{
  uint8_t  commandID;
  uint8_t  statusCode;
} ltlDefaultRspCmd_t;

/* for callback */
// Function pointer type to handle incoming messages.
// The return value of the plugin function will be
//  LTL_STATUS_SUCCESS - Supported and need default response
//  LTL_STATUS_FAILURE - Unsupported
//  LTL_STATUS_CMD_HAS_RSP - Supported and do not need default rsp
//  LTL_STATUS_INVALID_FIELD - Supported, but the incoming msg is wrong formatted
//  LTL_STATUS_INVALID_VALUE - Supported, but the request not achievable by the h/w
//  LTL_STATUS_MEMERROR - Supported but memory allocation fails
typedef LStatus_t (*ltlSpecificTrunckHdCB_t)( ltlApduMsg_t *ApduMsg );



/* 回调函数定义当   ltlAttrRec_t 属性记录中dataPtr为NULL是由用户提供数据
回调实现三个oper, LTL_OPER_LEN,LTL_OPER_READ,LTL_OPER_WRITE
由用户决定数据的存储,比如数据库
return LTL_STATUS_SUCCESS 成功
*/
typedef LStatus_t (*ltlReadWriteCB_t)( uint16_t trunkID, uint8_t nodeNO, uint16_t attrId, uint8_t oper,
                                       uint8_t *pValue, uint16_t *pLen );
/* 回调函数定义 由用户决定数据的授权,
回调实现两个oper, LTL_OPER_READ,LTL_OPER_WRITE
 @return  only LTL_STATUS_SUCCESS 成功  LTL_STATUS_NOT_AUTHORIZED 未授权
*/
typedef LStatus_t (*ltlAuthorizeCB_t)(ltlAttrRec_t *pAttr, uint8_t oper );


/*********************************************************************
 *              注册特定集下命令解析回调,用于解析集下命令
 * @brief       Add a trunk Library handler
 *
 * @param       starttrunkID -  trunk ID start
 * @param       endtrunkID -  trunk ID end
 * @param       pfnSpecificTrunkHdCB - function pointer to specific callback handler
 
 * @return      0 if OK
 */
LStatus_t ltl_registerPlugin(uint16_t starttrunkID,uint16_t endtrunkID,ltlSpecificTrunckHdCB_t pfnSpecificTrunkHdCB);
/*********************************************************************
 *              注册集下 指定节点的属性列表
 * @brief      
 * @param       trunkID -  trunk ID
 * @param       nodeNO -  node number
 * @param       numAttr -  attribute number 
 * @param       newAttrList[] - list of attrubute
 *
 * @return      0 if OK
 */
LStatus_t ltl_registerAttrList(uint16_t trunkID, uint8_t nodeNO, uint8_t numAttr,const ltlAttrRec_t newAttrList[] );
/*********************************************************************
 *              注册用户回调函数,处理属性和属性的授权  
 * @brief       Register the application's callback function to read/write
 *              attribute data, and authorize read/write operation.
 *
 *              Note: The pfnReadWriteCB callback function is only required
 *                    when the attribute data format is unknown to LTL. The
 *                    callback function gets called when the pointer 'dataPtr'
 *                    to the attribute value is NULL in the attribute database
 *                    registered with the LTL.  
 *              dataptr为NULL时,将调用此回调,用户处理此数据
 *
 *              Note: The pfnAuthorizeCB callback function is only required
 *                    when the Read/Write operation on an attribute requires
 *                    authorization (i.e., attributes with ACCESS_CONTROL_AUTH_READ
 *                    or ACCESS_CONTROL_AUTH_WRITE access permissions).
 *              授权回调,对
 *
 * @param       pfnReadWriteCB - function pointer to read/write routine
 * @param       pfnAuthorizeCB - function pointer to authorize read/write operation
 *
 * @return      LTL_SUCCESS if successful. LTL_FAILURE, otherwise.
 */
LStatus_t ltl_registerReadWriteCB(uint16_t trunkID, uint8_t nodeNO, 
                                ltlReadWriteCB_t pfnReadWriteCB, ltlAuthorizeCB_t pfnAuthorizeCB );




LStatus_t ltl_SendCommand(void *pAddr, uint16_t trunkID,uint8_t nodeNO,uint8_t seqNum, 
                                uint8_t specific, uint8_t direction, uint8_t disableDefaultRsp,
                                uint8_t cmd, uint8_t *cmdFormat,uint16_t cmdFormatLen);
LStatus_t ltl_SendReadReq(void *pAddr, uint16_t trunkID, uint8_t nodeNO,
                                uint8_t seqNum,uint8_t direction,
                                uint8_t disableDefaultRsp, ltlReadCmd_t *readCmd );
LStatus_t ltl_SendReadRsp(void *pAddr, uint16_t trunkID, uint8_t nodeNO,
                                uint8_t seqNum,uint8_t direction,
                                uint8_t disableDefaultRsp, ltlReadRspCmd_t *readRspCmd );
LStatus_t ltl_SendWriteRequest(void *pAddr, uint16_t trunkID, uint8_t nodeNO,
                                uint8_t seqNum,uint8_t direction,
                                uint8_t disableDefaultRsp, uint8_t cmd, ltlWriteCmd_t *writeCmd );
#define ltl_SendWriteReq(pAddr, trunkID, nodeNO, seqNum, direction, disableDefaultRsp, writeCmd ) \
                        ltl_SendWriteRequest(pAddr, trunkID, nodeNO, seqNum, direction, disableDefaultRsp, LTL_CMD_WRITE_ATTRIBUTES, writeCmd )
#define ltl_SendWriteRequUndivided(pAddr, trunkID, nodeNO, seqNum, direction, disableDefaultRsp, writeCmd ) \
                        ltl_SendWriteRequest(pAddr, trunkID, nodeNO, seqNum, direction, disableDefaultRsp, LTL_CMD_WRITE_ATTRIBUTES_UNDIVIDED, writeCmd )
                        
#define ltl_SendWriteReqNoRsp(pAddr, trunkID, nodeNO, seqNum, direction, disableDefaultRsp, writeCmd ) \
                        ltl_SendWriteRequest(pAddr, trunkID, nodeNO, seqNum, direction, disableDefaultRsp, LTL_CMD_WRITE_ATTRIBUTES_NORSP, writeCmd )
                        
LStatus_t ltl_SendwriteRsp( void *pAddr, uint16_t trunkID,uint8_t nodeNO,
                                 uint8_t seqNum , uint8_t direction,
                                 uint8_t disableDefaultRsp, ltlWriteRspCmd_t *writeRspCmd);
LStatus_t ltl_SendReportCmd( void *pAddr, uint16_t trunkID,uint8_t nodeNO,
                                 uint8_t seqNum , uint8_t direction,
                                 uint8_t disableDefaultRsp, ltlReportCmd_t *reportCmd);

LStatus_t ltl_SendDefaultRspCmd( void *pAddr, uint16_t trunkID,uint8_t nodeNO,
                                uint8_t seqNum, uint8_t direction,
                                uint8_t disableDefaultRsp, ltlDefaultRspCmd_t *defaultRspCmd);

void ltl_ProcessInApdu(MoIncomingMsgPkt_t *pkt);


uint8_t ltlFindAttrRec( uint16_t trunkID,  uint8_t nodeNO, uint16_t attrId, ltlAttrRec_t *pAttr );


void ltl_StrToAppString(char *pRawStr, char *pAppStr, uint8_t Applen );
void ltl_LongStrToAppString(char *pRawStr, char *pAppStr, uint16_t Applen );

#endif