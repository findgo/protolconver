#ifndef __LTL_H_
#define __LTL_H_

#include "ltldef.h"

// define in frame control field
//bit mask
#define LTL_FRAMECTL_TYPE_MASK             0x03
#define LTL_FRAMECTL_MCODE_MASK            0x04
#define LTL_FRAMECTL_DIR_MASK              0x08
#define LTL_FRAMECTL_DISALBE_DEFAULT_RSP_MASK  0x10
//subfield type
#define LTL_FRAMECTL_TYPE_PROFILE          0x00
#define LTL_FRAMECTL_TYPE_TRUNK_SPECIFIC   0x01
//subfield Manufacturer code
#define LTL_FRAMECTL_MANU_DSABLE 0
#define LTL_FRAMECTL_MANU_ENABLE 1
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
#define LTL_CMD_CONFIGURE_REPORTING             0x06
#define LTL_CMD_CONFIGURE_REPORTING_RSP         0x07
#define LTL_CMD_READ_CONFIGURE_REPORTING        0x08
#define LTL_CMD_READ_CONFIGURE_REPORTING_RSP    0x09
#define LTL_CMD_REPORT_ATTRIBUTES               0x0a
#define LTL_CMD_DEFAULT_RSP                     0x0b
#define LTL_CMD_DISCOVER_ATTRIBUTES             0x0c
#define LTL_CMD_DISCOVER_ATTRIBUTES_RSP         0x0d
//#define LTL_CMD_READ_ATTRIBUTES_STRUCTURED      0x0e
//#define LTL_CMD_WRITE_ATTRIBUTES_STRUCTURED     0x0f
//#define LTL_CMD_WRITE_ATTRIBUTES_STRUCTURED_RSP 0x10

#define LTL_CMD_PROFILE_MAX LTL_CMD_DISCOVER_ATTRIBUTES_RSP

/*** Data Types ***/
#define LTL_DATATYPE_NO_DATA                            0x00
#define LTL_DATATYPE_DATA8                              0x08
#define LTL_DATATYPE_DATA16                             0x09
#define LTL_DATATYPE_DATA32                             0x0b
#define LTL_DATATYPE_DATA64                             0x0f
#define LTL_DATATYPE_BOOLEAN                            0x10
#define LTL_DATATYPE_BITMAP8                            0x18
#define LTL_DATATYPE_BITMAP16                           0x19
#define LTL_DATATYPE_BITMAP32                           0x1b
#define LTL_DATATYPE_BITMAP64                           0x1f
#define LTL_DATATYPE_UINT8                              0x20
#define LTL_DATATYPE_UINT16                             0x21
#define LTL_DATATYPE_UINT32                             0x23
#define LTL_DATATYPE_UINT64                             0x27
#define LTL_DATATYPE_INT8                               0x28
#define LTL_DATATYPE_INT16                              0x29
#define LTL_DATATYPE_INT32                              0x2b
#define LTL_DATATYPE_INT64                              0x2f
#define LTL_DATATYPE_ENUM8                              0x30
#define LTL_DATATYPE_ENUM16                             0x31
//#define LTL_DATATYPE_SEMI_PREC                          0x38
#define LTL_DATATYPE_SINGLE_PREC                        0x39
#define LTL_DATATYPE_DOUBLE_PREC                        0x3a
#define LTL_DATATYPE_OCTET_STR                          0x41
#define LTL_DATATYPE_CHAR_STR                           0x42
#define LTL_DATATYPE_LONG_OCTET_STR                     0x43
#define LTL_DATATYPE_LONG_CHAR_STR                      0x44
#define LTL_DATATYPE_ARRAY                              0x48
//#define LTL_DATATYPE_STRUCT                             0x4c
//#define LTL_DATATYPE_SET                                0x50
//#define LTL_DATATYPE_BAG                                0x51
//#define LTL_DATATYPE_TOD                                0xe0
//#define LTL_DATATYPE_DATE                               0xe1
//#define LTL_DATATYPE_UTC                                0xe2
#define LTL_DATATYPE_TRUNK_ID                           0xe8
#define LTL_DATATYPE_ATTR_ID                            0xe9
//#define LTL_DATATYPE_BAC_OID                            0xea
//#define LTL_DATATYPE_IEEE_ADDR                          0xf0
#define LTL_DATATYPE_128_BIT_SEC_KEY                    0xf1
#define LTL_DATATYPE_UNKNOWN                            0xff



/*** Error Status Codes ***/
#define LTL_STATUS_SUCCESS                              0x00
#define LTL_STATUS_FAILURE                              0x01

#define LTL_STATUS_MEMERROR                             0x10
// 0x02-0x7D are reserved.
#define LTL_STATUS_NOT_AUTHORIZED                       0x7E
#define LTL_STATUS_MALFORMED_COMMAND                    0x80
#define LTL_STATUS_UNSUP_TRUNK_COMMAND                  0x81    //不支持集下命令
#define LTL_STATUS_UNSUP_GENERAL_COMMAND                0x82   //不支持profile命令
#define LTL_STATUS_UNSUP_MANU_TRUNK_COMMAND             0x83  // 不支持制造商集下的命令
#define LTL_STATUS_UNSUP_MANU_GENERAL_COMMAND           0x84  // 不支持制造商profile的命令
#define LTL_STATUS_INVALID_FIELD                        0x85
#define LTL_STATUS_UNSUPPORTED_ATTRIBUTE                0x86 
#define LTL_STATUS_INVALID_VALUE                        0x87
#define LTL_STATUS_READ_ONLY                            0x88
#define LTL_STATUS_INSUFFICIENT_SPACE                   0x89
#define LTL_STATUS_DUPLICATE_EXISTS                     0x8a
#define LTL_STATUS_NOT_FOUND                            0x8b
#define LTL_STATUS_UNREPORTABLE_ATTRIBUTE               0x8c
#define LTL_STATUS_INVALID_DATA_TYPE                    0x8d
#define LTL_STATUS_INVALID_SELECTOR                     0x8e
#define LTL_STATUS_WRITE_ONLY                           0x8f
#define LTL_STATUS_INCONSISTENT_STARTUP_STATE           0x90
#define LTL_STATUS_DEFINED_OUT_OF_BAND                  0x91
#define LTL_STATUS_INCONSISTENT                         0x92
#define LTL_STATUS_ACTION_DENIED                        0x93
#define LTL_STATUS_TIMEOUT                              0x94
#define LTL_STATUS_ABORT                                0x95
#define LTL_STATUS_INVALID_IMAGE                        0x96
#define LTL_STATUS_WAIT_FOR_DATA                        0x97
#define LTL_STATUS_NO_IMAGE_AVAILABLE                   0x98
#define LTL_STATUS_REQUIRE_MORE_IMAGE                   0x99
#define LTL_STATUS_SOFTWARE_FAILURE                     0x9a

// 0xc3-0xff are reserved.
#define LTL_STATUS_CMD_HAS_RSP                          0xff


/*** Attribute Access Control - bit masks ***/
#define ACCESS_CONTROL_READ                             0x01
#define ACCESS_CONTROL_WRITE                            0x02
#define ACCESS_CONTROL_COMMAND                          0x04
#define ACCESS_CONTROL_AUTH_READ                        0x10
#define ACCESS_CONTROL_AUTH_WRITE                       0x20


#define LTL_SUCCESS  0x00
#define LTL_FAILURE  0x01  
#define LTL_MEMERROR 0x02


// Used by ltlReadWriteCB_t callback function
#define LTL_OPER_LEN                                    0x00 // Get length of attribute value to be read
#define LTL_OPER_READ                                   0x01 // Read attribute value
#define LTL_OPER_WRITE                                  0x02 // Write new attribute value

/*********************************************************************
 * MACROS
 */
#define ltl_IsProfileCmd( a )           ( (a) == LTL_FRAMECTL_TYPE_PROFILE )
#define ltl_IsTrunkCmd( a )             ( (a) == LTL_FRAMECTL_TYPE_TRUNK_SPECIFIC )

#define ltl_IsMancodeEnable(a)          ( (a) == LTL_FRAMECTL_MANU_ENABLE )
#define ltl_IsMancodeDisable(a)         ( (a) == LTL_FRAMECTL_MANU_DSABLE )

#define ltl_IsRcvbyServerCmd( a )       ( (a) == LTL_FRAMECTL_DIR_CLIENT_SERVER )
#define ltl_IsRcvbyClientCmd( a )       ( (a) == LTL_FRAMECTL_DIR_SERVER_CLIENT )


// LTL header - frame control field
typedef struct
{
    uint8_t type;
    uint8_t manuSpecific;
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
    uint16_t    manuCode;
    uint8_t     commandID;
    ltlFrameHdrctl_t fc;
} ltlFrameHdr_t;

typedef struct
{
    uint8_t *refer;
    uint16_t apduLength;
    uint8_t *apduData;
}MoIncomingMsgPkt_t;

typedef struct 
{
    MoIncomingMsgPkt_t *pkt;
    ltlFrameHdr_t hdr;
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

// Function pointer type to handle incoming messages.
//   msg - incoming message
typedef LStatus_t (*ltlInHdlr_t)( ltlApduMsg_t *pInHdlrMsg );


/*------------------------for application-----------------------------------------------*/

// Attribute record
typedef struct
{
  uint16_t  attrId;         // Attribute ID
  uint8_t   dataType;       // Data Type 
  uint8_t   accessControl;  // Read/write acess - bit field
  void    *dataPtr;       // Pointer to data field
} ltlAttrRec_t;



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
// Default Response Command format
typedef struct
{
  uint8_t  commandID;
  uint8_t  statusCode;
} ltlDefaultRspCmd_t;

LStatus_t ltl_SendCommand(uint8_t *refer,        uint16_t trunkID,uint8_t nodeNO, uint8_t cmd, uint8_t specific, 
                            uint8_t direction, uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
                            uint8_t *cmdFormat,uint16_t cmdFormatLen);


LStatus_t ltl_SendDefaultRspCmd( uint8_t *refer, uint16_t trunkID,uint8_t nodeNO,
                                 ltlDefaultRspCmd_t *defaultRspCmd, uint8_t direction,
                                 uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum );
LStatus_t ltl_SendReadRsp( uint8_t *refer, uint16_t trunkID,uint8_t nodeNO,
                                 ltlReadRspCmd_t *readRspCmd, uint8_t direction,
                                 uint8_t disableDefaultRsp, uint8_t seqNum );


#endif
