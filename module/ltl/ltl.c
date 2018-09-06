

#include "ltl.h"
#include "memalloc.h"
#include "prefix.h"



/*********************************************************************
 * MACROS
 */
/*** Frame Control ***/
#define ltl_FCType( a )               ( (a) & LTL_FRAMECTL_TYPE_MASK )
#define ltl_FCManuSpecific( a )       ( (a) & LTL_FRAMECTL_MCODE_MASK )
#define ltl_FCDirection( a )          ( (a) & LTL_FRAMECTL_DIR_MASK)
#define ltl_FCDisableDefaultRsp( a )  ( (a) & LTL_FRAMECTL_DISALBE_DEFAULT_RSP_MASK )

/*** Attribute Access Control ***/
#define ltl_AccessCtrlRead( a )       ( (a) & ACCESS_CONTROL_READ )
#define ltl_AccessCtrlWrite( a )      ( (a) & ACCESS_CONTROL_WRITE )
#define ltl_AccessCtrlCmd( a )        ( (a) & ACCESS_CONTROL_COMMAND )
#define ltl_AccessCtrlAuthRead( a )   ( (a) & ACCESS_CONTROL_AUTH_READ )
#define ltl_AccessCtrlAuthWrite( a )  ( (a) & ACCESS_CONTROL_AUTH_WRITE )

// Commands that have corresponding responses
#define LTL_PROFILE_CMD_HAS_RSP( cmd )  ( (cmd) == LTL_CMD_READ_ATTRIBUTES            || \
                                        (cmd) == LTL_CMD_WRITE_ATTRIBUTES           || \
                                        (cmd) == LTL_CMD_WRITE_ATTRIBUTES_UNDIVIDED || \
                                        (cmd) == LTL_CMD_CONFIGURE_REPORTING   || \
                                        (cmd) == LTL_CMD_READ_CONFIGURE_REPORTING || \
                                        (cmd) == LTL_CMD_DISCOVER_ATTRIBUTES        || \
                                        (cmd) == LTL_CMD_DEFAULT_RSP ) // exception

typedef LStatus_t (*ltlReadWriteCB_t)( uint16_t trunkID, uint8_t nodeNO, uint16_t attrId, uint8_t oper,
                                       uint8_t *pValue, uint16_t *pLen );

typedef LStatus_t (*ltlAuthorizeCB_t)(ltlAttrRec_t *pAttr, uint8_t oper );


typedef void *(*ltlParseInProfileCmd_t)( uint8_t *pbuf,uint16_t datalen );
typedef uint8_t (*ltlProcessInProfileCmd_t)( ltlApduMsg_t *ApduMsg );

typedef uint16_t (*ltlprefixsizefn_t)(uint8_t *refer);
typedef uint8_t *(*ltlprefixBuildHdrfn_t)(uint8_t *refer,uint8_t *pbuf);

typedef struct
{
    ltlParseInProfileCmd_t   pfnParseInProfile;
    ltlProcessInProfileCmd_t pfnProcessInProfile;
} ltlCmdItems_t;

typedef struct ltlLibPlugin_s
{
    uint16_t    starttrunkID;         //  start trunk ID
    uint16_t    endtrunkID;         //  end trunk ID
    ltlInHdlr_t pfnIncomingHdlr;    // function to handle incoming message
    void *next;
} ltlLibPlugin_t;

// Attribute record list item
typedef struct ltlAttrRecsList_s
{
    uint16_t            trunkID;      // Used to link it into the trunk descriptor
    uint8_t             nodeNO; // Number of the following records
    uint8_t             numAttributes;
    ltlAttrRec_t        *attrs;        // attribute record
    ltlReadWriteCB_t    pfnReadWriteCB;// Read or Write attribute value callback function
    ltlAuthorizeCB_t    pfnAuthorizeCB;// Authorize Read or Write operation   
    void *next;
} ltlAttrRecsList_t;


// local function
static void *ltlParseInReadCmd( uint8_t *pbuf,uint16_t datalen );
static uint8_t ltlProcessInReadCmd(ltlApduMsg_t *ApduMsg);
static uint8_t *ltlSerializeData( uint8_t dataType, void *attrData, uint8_t *buf );
static uint8_t *ltlParseHdr(ltlFrameHdr_t *hdr,uint8_t *pDat);
static void ltlEncodeHdr( ltlFrameHdr_t *hdr, uint16_t trunkID,uint8_t nodeNO, uint8_t cmd, uint8_t specific, uint8_t direction,
                           uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum);
static uint8_t *ltlBuildHdr( ltlFrameHdr_t *hdr, uint8_t *pDat );
static uint8_t ltlHdrSize(ltlFrameHdr_t *hdr);
static ltlLibPlugin_t *ltlFindPlugin( uint16_t trunkID );
static ltlAttrRecsList_t *ltlFindAttrRecsList(uint16_t trunkID, uint8_t nodeNO);
static uint8_t ltlFindAttrRec( uint16_t trunkID,  uint8_t nodeNO, uint16_t attrId, ltlAttrRec_t *pAttr );


static ltlReadWriteCB_t ltlGetReadWriteCB(uint16_t trunkID, uint8_t nodeNO);
static LStatus_t ltlAuthorizeRead(uint16_t trunkID, uint8_t nodeNO, ltlAttrRec_t *pAttr );

// local variable
static ltlLibPlugin_t *libplugins = NULL;
static ltlAttrRecsList_t *attrList = NULL;

static ltlCmdItems_t ltlCmdTable[] = 
{
    {ltlParseInReadCmd, ltlProcessInReadCmd},
};



/*********************************************************************
 * @fn         
 *              注册特定集下命令解析回调,用于解析集下命令
 * @brief       Add a trunk Library handler
 *
 * @param       starttrunkID -  trunk ID
 * @param       endtrunkID -  trunk ID
 * @param       pfnHdlr - function pointer to incoming message handler
 *
 * @return      0 if OK
 */
// ok
LStatus_t ltl_registerPlugin(uint16_t starttrunkID,uint16_t endtrunkID,ltlInHdlr_t pfnInHdlr)
{
    ltlLibPlugin_t *pNewItem;
    ltlLibPlugin_t *pLoop;

    // Fill in the new profile list
    pNewItem = mo_malloc( sizeof( ltlLibPlugin_t ) );
    if ( pNewItem == NULL ){
        return LTL_MEMERROR;
    }

    // Fill in the plugin record.
    pNewItem->starttrunkID = starttrunkID;
    pNewItem->endtrunkID = endtrunkID;
    pNewItem->pfnIncomingHdlr = pfnInHdlr;
    pNewItem->next = (ltlLibPlugin_t *)NULL;
    
    // Find spot in list
    if ( libplugins == NULL ){
        libplugins = pNewItem;
    }
    else{
        // Look for end of list
        pLoop = libplugins;
        while ( pLoop->next != NULL )
        {
            pLoop = pLoop->next;
        }

        // Put new item at end of list
        pLoop->next = pNewItem;
    }

    return LTL_SUCCESS;
}

/*********************************************************************
 * @fn         
 *              注册集下 指定节点的属性列表
 * @brief      
 *
 * @param       trunkID -  trunk ID
 * @param       nodeNO -  node number
 * @param       numAttr -  attribute number 
 * @param       newAttrList[] - list of attrubute
 *
 * @return      0 if OK
 */
// ok
LStatus_t ltl_registerAttrList(uint16_t trunkID, uint8_t nodeNO, uint8_t numAttr, ltlAttrRec_t newAttrList[] )
{
    ltlAttrRecsList_t *pNewItem;
    ltlAttrRecsList_t *pLoop;

    // Fill in the new profile list
    pNewItem = mo_malloc( sizeof( ltlAttrRecsList_t ) );
    if ( pNewItem == NULL){
        return LTL_MEMERROR;
    }

    pNewItem->trunkID = trunkID;
    pNewItem->nodeNO = nodeNO;
    pNewItem->numAttributes = numAttr;
    pNewItem->attrs = newAttrList;
    pNewItem->pfnReadWriteCB = NULL;
    pNewItem->pfnAuthorizeCB = NULL;
    pNewItem->next = (void *)NULL;
    
    // Find spot in list
    if ( attrList == NULL ) {
        attrList = pNewItem;
    }
    else{
        // Look for end of list
        pLoop = attrList;
        while ( pLoop->next != NULL )
        {
            pLoop = pLoop->next;
        }

        // Put new item at end of list
        pLoop->next = pNewItem;
    }

    return LTL_SUCCESS;
}

/*********************************************************************
 * @fn       注册用户回调函数   
 *
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
LStatus_t LTL_registerReadWriteCB(uint16_t trunkID, uint8_t nodeNO, 
                                ltlReadWriteCB_t pfnReadWriteCB, ltlAuthorizeCB_t pfnAuthorizeCB )
{
    ltlAttrRecsList_t *pRec = ltlFindAttrRecsList(trunkID, nodeNO);

    if ( pRec != NULL ){
        pRec->pfnReadWriteCB = pfnReadWriteCB;
        pRec->pfnAuthorizeCB = pfnAuthorizeCB;

        return ( LTL_SUCCESS );
    }

    return ( LTL_FAILURE );
}

/*********************************************************************
 * @fn      
 *
 * @brief   Used to send Profile and trunk Specific Command messages.
 *
 *          NOTE: The calling application is responsible for incrementing
 *                the Sequence Number.
 *
 * @param   trunkID - trunk ID
 * @param   nodeNO -  node number
 * @param   cmd - command ID
 * @param   specific - whether the command is trunk Specific
 * @param   direction - client/server direction of the command
 * @param   disableDefaultRsp - disable Default Response command
 * @param   manuCode - manufacturer code for proprietary extensions to a profile
 * @param   seqNumber - identification number for the transaction
 * @param   cmdFormatLen - length of the command to be sent
 * @param   cmdFormat - command to be sent
 *
 * @return  ZSuccess if OK
 */

LStatus_t ltl_SendCommand(uint8_t *refer,        uint16_t trunkID,uint8_t nodeNO, uint8_t cmd, uint8_t specific, 
                            uint8_t direction, uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum,
                            uint8_t *cmdFormat,uint16_t cmdFormatLen)
{
    ltlFrameHdr_t hdr;
    uint8_t *msgbuf;
    uint8_t *pbuf;
    uint16_t msglen;
    uint16_t prefixlen;
    
    ltlEncodeHdr(&hdr, trunkID, nodeNO, cmd, specific, direction, disableDefaultRsp, manuCode, seqNum);

    msglen = ltlHdrSize(&hdr);

    //获得前置预留长度
    prefixlen = ltlprefixsize(refer);
    msglen += prefixlen + cmdFormatLen;

    msgbuf = (uint8_t *)mo_malloc(msglen);
    if(!msgbuf){
        return LTL_STATUS_MEMERROR;
    }
 
    pbuf = ltlPrefixBuildHdr(refer, msgbuf);
    pbuf = ltlBuildHdr(&hdr, pbuf);
    memcpy(pbuf, cmdFormat, cmdFormatLen);

    ltlrequest(msgbuf, msglen);
    mo_free(msgbuf);
    
    return LTL_STATUS_SUCCESS;
}

/*********************************************************************
 * @fn      
 *
 * @brief   Parse header of the ltl format
 *
 * @param   hdr -  out ----place to put the frame head  information
 * @param   pDat - in  ---- incoming apdu buffer to parse
 *
 * @return  pointer past the header
 */
//ok
static uint8_t *ltlParseHdr(ltlFrameHdr_t *hdr,uint8_t *pDat)
{
    memset((uint8_t *)hdr,0,sizeof(ltlFrameHdr_t));

    hdr->trunkID = BUILD_UINT16(*pDat, *(pDat +1));
    pDat += 2;
    hdr->transSeqNum = *pDat;
    pDat++;
    hdr->nodeNo = *pDat;
    pDat++;

    hdr->fc.type = ltl_FCType(*pDat);
    hdr->fc.manuSpecific = ltl_FCManuSpecific(*pDat) ? LTL_FRAMECTL_MANU_ENABLE : LTL_FRAMECTL_MANU_DSABLE;
    hdr->fc.direction = ltl_FCDirection(*pDat) ? LTL_FRAMECTL_DIR_SERVER_CLIENT : LTL_FRAMECTL_DIR_CLIENT_SERVER;
    hdr->fc.disableDefaultRsp = ltl_FCDisableDefaultRsp(*pDat) ? LTL_FRAMECTL_DIS_DEFAULT_RSP_ON : LTL_FRAMECTL_DIS_DEFAULT_RSP_OFF;
    pDat++;

    if(hdr->fc.manuSpecific){
        hdr->manuCode = BUILD_UINT16(*pDat, *(pDat + 1));
        pDat += 2;
    }

    hdr->commandID = *pDat++;
    
    return (pDat);
}
static void ltlEncodeHdr( ltlFrameHdr_t *hdr, uint16_t trunkID,uint8_t nodeNO, uint8_t cmd, uint8_t specific, uint8_t direction,
                           uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum)
{
    hdr->trunkID = trunkID;
    
    //Transaction Sequence Number
    hdr->transSeqNum = seqNum;
    // node number
    hdr->nodeNo = nodeNO;
    // Add the Trunk's command ID
    hdr->commandID = cmd;
    
    // Build the Frame Control byte
    hdr->fc.type = specific ? LTL_FRAMECTL_TYPE_TRUNK_SPECIFIC : LTL_FRAMECTL_TYPE_PROFILE;
    hdr->fc.manuSpecific = manuCode ? LTL_FRAMECTL_MANU_ENABLE : LTL_FRAMECTL_MANU_DSABLE;
    hdr->fc.direction = direction ? LTL_FRAMECTL_DIR_SERVER_CLIENT : LTL_FRAMECTL_DIR_CLIENT_SERVER;
    hdr->fc.disableDefaultRsp=  disableDefaultRsp ? LTL_FRAMECTL_DIS_DEFAULT_RSP_ON : LTL_FRAMECTL_DIS_DEFAULT_RSP_OFF;

    // Add the manfacturer code
    if ( hdr->fc.manuSpecific ){
        hdr->manuCode = manuCode;
    }
}

/*********************************************************************
 * @fn     
 *
 * @brief   Build header of the ltL format
 *
 * @param   hdr - in ---- header information
 * @param   pDat -out ---- outgoing header space
 *
 * @return  pointer past the header
 */

static uint8_t *ltlBuildHdr( ltlFrameHdr_t *hdr, uint8_t *pDat )
{
    *pDat++ = LO_UINT16(hdr->trunkID);
    *pDat++ = HI_UINT16(hdr->trunkID);
    //Transaction Sequence Number
    *pDat++ = hdr->transSeqNum;
    // node number
    *pDat++ = hdr->nodeNo;

    // Build the Frame Control byte
    *pDat = hdr->fc.type;
    *pDat |= hdr->fc.manuSpecific << 2;
    *pDat |= hdr->fc.direction << 3;
    *pDat |= hdr->fc.disableDefaultRsp << 4;
    pDat++;  // move past the frame control field

    // Add the manfacturer code
    if ( hdr->fc.manuSpecific ){
        *pDat++ = LO_UINT16( hdr->manuCode );
        *pDat++ = HI_UINT16( hdr->manuCode );
    }
    
    // Add the Trunk's command ID
    *pDat++ = hdr->commandID;

    // Should point to the frame payload
    return ( pDat );
}
static uint8_t ltlHdrSize(ltlFrameHdr_t *hdr)
{
    uint8_t size = (2 + 1 + 1 + 1 + 1); //trunkID + seqnum + nodeNO + frame control + cmdID

  // Add the manfacturer code
    if ( hdr->fc.manuSpecific ){
        size += 2;
    }

    return ( size );
}

//处理进来的apdu帧,本函数只处理APDU
void ltlProcessInApdu(MoIncomingMsgPkt_t *pkt)
{
    LStatus_t status = LTL_STATUS_SUCCESS;
    ltlApduMsg_t ApduMsg;
    ltlLibPlugin_t *pInPlugin;
    ltlDefaultRspCmd_t defaultRspCmd;

    //error 
    if(pkt->apduLength == 0){
        return;
    }

    //parse frame head
    ApduMsg.pkt = pkt;
    ApduMsg.pdata = ltlParseHdr(&(ApduMsg.hdr),pkt->apduData);
    ApduMsg.datalen = pkt->apduLength;
    ApduMsg.datalen -= (uint16_t)(ApduMsg.pdata - pkt->apduData); // 通过指针偏移计算apdu帧头数据长度

    // process frame head    
    //foundation type message,
    if(ltl_IsProfileCmd(ApduMsg.hdr.fc.type)){
        if(ltl_IsMancodeEnable(ApduMsg.hdr.fc.manuSpecific)){            
            // 自定义 profile下的制造商命令
            //TODO: realize after
            status = LTL_STATUS_UNSUP_MANU_GENERAL_COMMAND;
        }
        else if((ApduMsg.hdr.commandID <= LTL_CMD_PROFILE_MAX) &&
            (ltlCmdTable[ApduMsg.hdr.commandID].pfnParseInProfile != NULL)){
            // parse foundation corresponding command 
            ApduMsg.attrCmd = ltlCmdTable[ApduMsg.hdr.commandID].pfnParseInProfile(ApduMsg.pdata,ApduMsg.datalen);
            if((ApduMsg.attrCmd != NULL) && (ltlCmdTable[ApduMsg.hdr.commandID].pfnProcessInProfile != NULL)){
                //and then process corresponding appliction data domain
                if(ltlCmdTable[ApduMsg.hdr.commandID].pfnProcessInProfile(&ApduMsg) == FALSE){
                    // can not find attribute in the table
                }
            }

            if(ApduMsg.attrCmd)
                mo_free(ApduMsg.attrCmd);

            if(LTL_PROFILE_CMD_HAS_RSP(ApduMsg.hdr.commandID))
                return; // we are done in ltlProcessCmd()
            
        }
        else {
            status = LTL_STATUS_UNSUP_GENERAL_COMMAND;
        }
    }
    // Not a foundation type message,
    else{ //ltl_IsTrunkCmd(ApduMsg.hdr.fc.type)
        // it is specific to the trunk id command
        //find trunk id 
        pInPlugin = ltlFindPlugin(ApduMsg.hdr.trunkID);
        if(pInPlugin && pInPlugin->pfnIncomingHdlr){
                  // The return value of the plugin function will be
            //  LTL_STATUS_SUCCESS - Supported and need default response
            //  LTL_STATUS_FAILURE - Unsupported
            //  LTL_STATUS_CMD_HAS_RSP - Supported and do not need default rsp
            //  LTL_STATUS_INVALID_FIELD - Supported, but the incoming msg is wrong formatted
            //  LTL_STATUS_INVALID_VALUE - Supported, but the request not achievable by the h/w
            //  LTL_STATUS_MEMERROR - Supported but memory allocation fails
            
            status = pInPlugin->pfnIncomingHdlr(&ApduMsg);
            if(status == LTL_STATUS_CMD_HAS_RSP)
                return; // we are done in pfnIncomingHdlr
        }

        if(status == LTL_STATUS_FAILURE){           
            status = ApduMsg.hdr.fc.manuSpecific ? LTL_STATUS_UNSUP_MANU_TRUNK_COMMAND :  LTL_STATUS_UNSUP_TRUNK_COMMAND;
        }
    }

    if(ApduMsg.hdr.fc.disableDefaultRsp == LTL_FRAMECTL_DIS_DEFAULT_RSP_OFF ){
        defaultRspCmd.commandID = ApduMsg.hdr.commandID;
        defaultRspCmd.statusCode = status;
        // send default response
        ltl_SendDefaultRspCmd(pkt->refer, ApduMsg.hdr.trunkID, ApduMsg.hdr.nodeNo, 
                            &defaultRspCmd,  LTL_FRAMECTL_DIR_SERVER_CLIENT,  TRUE,  
                            ApduMsg.hdr.manuCode, ApduMsg.hdr.transSeqNum);
    }
}
/*********************************************************************
 * @fn         
 *          
 * @brief       find a trunk Library pointer
 *
 * @param       trunkID -  trunk ID
 * @param       pfnHdlr - function 
 *
 * @return      pointer to trunk lib,if NULL not find
 */
// ok
static ltlLibPlugin_t *ltlFindPlugin( uint16_t trunkID )
{
    ltlLibPlugin_t *pLoop = libplugins;

    while ( pLoop != NULL )
    {
        if ( (trunkID >= pLoop->starttrunkID) &&  (trunkID <= pLoop->endtrunkID ) ){
            return ( pLoop );
        }

        pLoop = pLoop->next;
    }

    return ( (ltlLibPlugin_t *)NULL );
}
/*********************************************************************
 * @fn      
 *
 * @brief   Find the right attribute record list for an trunkID and nodeNumber
 *
 * @param   trunkID  -  look for
 * @param   nodeNO   -  look for
 *
 * @return  pointer to record list, NULL if not found
 */

static ltlAttrRecsList_t *ltlFindAttrRecsList(uint16_t trunkID, uint8_t nodeNO)
{
    ltlAttrRecsList_t *pLoop = attrList;

    while ( pLoop != NULL )
    {
        if ( pLoop->nodeNO == nodeNO && pLoop->trunkID == trunkID){
            return ( pLoop );
        }

        pLoop = (ltlAttrRecsList_t *)pLoop->next;
    }

    return ( NULL );
}
/*********************************************************************
 * @fn      
 *
 * @brief   Find the attribute record that matchs the parameters
 *
 * @param   trunkID - trunk ID
 * @param   nodeNO - node number
 * @param   attrId - attribute looking for
 * @param   pAttr - attribute record to be returned
 *
 * @return  TRUE if record found. FALSE, otherwise.
 */
static uint8_t ltlFindAttrRec( uint16_t trunkID,  uint8_t nodeNO, uint16_t attrId, ltlAttrRec_t *pAttr )
{
    uint8_t i;
    ltlAttrRecsList_t *pRec = ltlFindAttrRecsList( trunkID, nodeNO);

    if ( pRec == NULL ){
        return ( FALSE );
    }
    
    for ( i = 0; i < pRec->numAttributes; i++ ){
        if (pRec->attrs[i].attrId == attrId ){
            *pAttr = pRec->attrs[i]; // TODO check?
    
            return ( TRUE );
        }
    }
    
    return FALSE;
}

uint8_t ltlGetDataTypeLength( uint8_t dataType )
{
    uint8_t len;

    switch ( dataType ){
        case LTL_DATATYPE_DATA8:
        case LTL_DATATYPE_BOOLEAN:
        case LTL_DATATYPE_BITMAP8:
        case LTL_DATATYPE_INT8:
        case LTL_DATATYPE_UINT8:
        case LTL_DATATYPE_ENUM8:
            len = 1;
        break;

        case LTL_DATATYPE_DATA16:
        case LTL_DATATYPE_BITMAP16:
        case LTL_DATATYPE_UINT16:
        case LTL_DATATYPE_INT16:
        case LTL_DATATYPE_ENUM16:
//        case LTL_DATATYPE_SEMI_PREC:
//        case LTL_DATATYPE_TRUNK_ID:
//        case LTL_DATATYPE_ATTR_ID:
            len = 2;
        break;

        case LTL_DATATYPE_DATA32:
        case LTL_DATATYPE_BITMAP32:
        case LTL_DATATYPE_UINT32:
        case LTL_DATATYPE_INT32:
        case LTL_DATATYPE_SINGLE_PREC:
//        case LTL_DATATYPE_TOD:
//        case LTL_DATATYPE_DATE:
//        case LTL_DATATYPE_UTC:
//        case LTL_DATATYPE_BAC_OID:
            len = 4;
        break;

        case LTL_DATATYPE_DOUBLE_PREC:
//        case LTL_DATATYPE_IEEE_ADDR:
        case LTL_DATATYPE_UINT64:
        case LTL_DATATYPE_INT64:
            len = 8;
        break;

        case LTL_DATATYPE_128_BIT_SEC_KEY:
            len = 16;
        break;

        case LTL_DATATYPE_NO_DATA:
        case LTL_DATATYPE_UNKNOWN:
        // Fall through

        default:
            len = 0;
        break;
    }

    return ( len );
}

/*********************************************************************
 * @fn      LTLGetAttrDataLength
 *
 * @brief   Return the length of the attribute.
 *
 * @param   dataType - data type
 * @param   pData - pointer to data
 *
 * @return  returns atrribute length
 */
uint16_t ltlGetAttrDataLength( uint8_t dataType, uint8_t *pData )
{
    uint16_t dataLen = 0;

    if ( dataType == LTL_DATATYPE_LONG_CHAR_STR || dataType == LTL_DATATYPE_LONG_OCTET_STR ) {
        dataLen = BUILD_UINT16( pData[0], pData[1] ) + 2; // long string length + 2 for length field
    }
    else if ( dataType == LTL_DATATYPE_CHAR_STR || dataType == LTL_DATATYPE_OCTET_STR ){
        dataLen = *pData + 1; // string length + 1 for length field
    }
    else{
        dataLen = ltlGetDataTypeLength( dataType );
    }

    return ( dataLen );
}

/*********************************************************************
 * @fn      
 *
 * @brief   Parse the "Profile" Read Commands
 *
 *      NOTE: THIS FUNCTION ALLOCATES THE RETURN BUFFER, SO THE CALLING
 *            FUNCTION IS RESPONSIBLE TO FREE THE MEMORY.
 *
 * @param   pCmd - pointer to incoming data to parse
 *
 * @return  pointer to the parsed command structure
 */
 //ok
static void *ltlParseInReadCmd( uint8_t *pbuf,uint16_t datalen )
{
    ltlReadCmd_t *readCmd;
    uint8_t i;    
    
    readCmd = (ltlReadCmd_t *)mo_malloc( sizeof ( ltlReadCmd_t ) + datalen );
    if ( readCmd == NULL ){
        return NULL;
   }
    
    readCmd->numAttr = datalen / 2; // Atrribute ID number
    for ( i = 0; i < readCmd->numAttr; i++ ){
        readCmd->attrID[i] = BUILD_UINT16( pbuf[0], pbuf[1] );
        pbuf += 2;
    }

    return ( (void *)readCmd );
}
/*********************************************************************
 * @fn      
 *
 * @brief   Process the "Profile" Read Command
 *
 * @param   ApduMsg - apdu message to process
 *
 * @return  TRUE if command processed. FALSE, otherwise.
 */
 static uint8_t ltlProcessInReadCmd(ltlApduMsg_t *ApduMsg)
{
    ltlReadCmd_t *readCmd;
    ltlReadRspCmd_t *readRspCmd;
    ltlAttrRec_t attrRec;
    ltlReadRspStatus_t *statusRec;
    uint16_t len;
    uint8_t i;

    readCmd = (ltlReadCmd_t *)ApduMsg->attrCmd;

    // calculate the length of the response status record
    len = sizeof( ltlReadRspCmd_t ) + (readCmd->numAttr * sizeof( ltlReadRspStatus_t ));

    readRspCmd = ( ltlReadRspCmd_t *)mo_malloc( len );
    if ( readRspCmd == NULL ){
        return FALSE; // EMBEDDED RETURN
    }

    // 对每一个属性进行读处理,并准备好response 
    readRspCmd->numAttr = readCmd->numAttr;
    for ( i = 0; i < readCmd->numAttr; i++ ){
        statusRec = &(readRspCmd->attrList[i]);

        statusRec->attrID = readCmd->attrID[i];

        if ( ltlFindAttrRec( ApduMsg->hdr.trunkID, ApduMsg->hdr.nodeNo, readCmd->attrID[i], &attrRec ) ){
            if ( ltl_AccessCtrlRead( attrRec.accessControl ) ) {
                statusRec->status = ltlAuthorizeRead( ApduMsg->hdr.trunkID, ApduMsg->hdr.nodeNo, &attrRec );
                if ( statusRec->status == LTL_STATUS_SUCCESS ){
                    statusRec->data = attrRec.dataPtr;
                    statusRec->dataType = attrRec.dataType;
                }
            }
            else{
                statusRec->status = LTL_STATUS_WRITE_ONLY;
            }
        }
        else{
            statusRec->status = LTL_STATUS_UNSUPPORTED_ATTRIBUTE;
        }
    }

    // Build and send Read Response command
    ltl_SendReadRsp( ApduMsg->pkt->refer,ApduMsg->hdr.trunkID, ApduMsg->hdr.nodeNo,
                    readRspCmd, LTL_FRAMECTL_DIR_SERVER_CLIENT,
                    true, ApduMsg->hdr.transSeqNum );
    mo_free( readRspCmd );

    return TRUE;
}


static ltlReadWriteCB_t ltlGetReadWriteCB(uint16_t trunkID, uint8_t nodeNO)
{
  ltlAttrRecsList_t *pRec = ltlFindAttrRecsList( trunkID, nodeNO );

  if ( pRec != NULL )
  {
    return ( pRec->pfnReadWriteCB );
  }

  return ( NULL );
}
/*********************************************************************
 * @fn      
 *
 * @brief   Get the Read/Write Authorization callback function pointer
 *          for a given endpoint.
 *
 * @param   trunkID -
 * @param   nodeNO - 
 *
 * @return  Authorization CB, NULL if not found
 */
static ltlAuthorizeCB_t ltlGetAuthorizeCB(uint16_t trunkID, uint8_t nodeNO)
{
    ltlAttrRecsList_t *pRec = ltlFindAttrRecsList( trunkID, nodeNO );

    if ( pRec != NULL ){
        return ( pRec->pfnAuthorizeCB );
    }

    return ( NULL );
}
static uint16_t ltlGetAttrDataLengthUsingCB( uint16_t trunkID,uint8_t nodeNO,  uint16_t attrId )
{
    uint16_t dataLen = 0;
    ltlReadWriteCB_t pfnReadWriteCB = ltlGetReadWriteCB( trunkID, nodeNO );

    if ( pfnReadWriteCB != NULL ){
        // Only get the attribute length
        (*pfnReadWriteCB)( trunkID, nodeNO, attrId, LTL_OPER_LEN, NULL, &dataLen );
    }

    return ( dataLen );
}

static LStatus_t ltlReadAttrDataUsingCB( uint16_t trunkID,uint8_t nodeNO,  uint16_t attrId ,
                                         uint8_t *pAttrData, uint16_t *pDataLen )
{
    ltlReadWriteCB_t pfnReadWriteCB = ltlGetReadWriteCB( trunkID, nodeNO );

    if ( pDataLen != NULL ){
        *pDataLen = 0; // Always initialize it to 0
    }

    if ( pfnReadWriteCB != NULL )
    {
        // Read the attribute value and its length
        return ( (*pfnReadWriteCB)( trunkID, nodeNO, attrId, LTL_OPER_READ, pAttrData, pDataLen ) );
    }

    return ( LTL_STATUS_SOFTWARE_FAILURE );
}
/*********************************************************************
 * @fn      
 *
 * @brief   Use application's callback to authorize a Read operation
 *          on a given attribute.
 *
 * @param   trunkID - 
 * @param   nodeNO - 
 * @param   pAttr - pointer to attribute
 *
 * @return  LTL_STATUS_SUCCESS: Operation authorized
 *          LTL_STATUS_NOT_AUTHORIZED: Operation not authorized
 */
static LStatus_t ltlAuthorizeRead(uint16_t trunkID, uint8_t nodeNO, ltlAttrRec_t *pAttr )
{
    ltlAuthorizeCB_t pfnAuthorizeCB;

    if ( ltl_AccessCtrlAuthRead( pAttr->accessControl ) ){
        pfnAuthorizeCB = ltlGetAuthorizeCB( trunkID,nodeNO );

        if ( pfnAuthorizeCB != NULL ){
            return ( (*pfnAuthorizeCB)(pAttr, LTL_OPER_READ ) );
        }
    }

    return ( LTL_STATUS_SUCCESS );
}



LStatus_t ltl_SendReadRsp( uint8_t *refer, uint16_t trunkID,uint8_t nodeNO,
                                 ltlReadRspCmd_t *readRspCmd, uint8_t direction,
                                 uint8_t disableDefaultRsp, uint8_t seqNum )
{
    uint8_t *buf;
    uint8_t *pBuf;
    uint16_t len = 0;
    uint16_t dataLen;
    LStatus_t status;
    uint8_t i;
    ltlReadRspStatus_t *statusRec;

    // calculate the size of the command
    for ( i = 0; i < readRspCmd->numAttr; i++ ) {
        statusRec = &(readRspCmd->attrList[i]);

        len += 2 + 1; // Attribute ID + Status

        if ( statusRec->status == LTL_STATUS_SUCCESS ){
            len++; // Attribute Data Type length

            // Attribute Data length
            if ( statusRec->data != NULL ){
                len += ltlGetAttrDataLength( statusRec->dataType, statusRec->data );
            }
            else{
                len += ltlGetAttrDataLengthUsingCB( trunkID, nodeNO, statusRec->attrID );
            }
        }
    }

    buf = mo_malloc( len );
    if ( buf == NULL ){
        return LTL_STATUS_MEMERROR;
    }
    
    // Load the buffer - serially
    pBuf = buf;
    for ( i = 0; i < readRspCmd->numAttr; i++ ){
        statusRec = &(readRspCmd->attrList[i]);

        *pBuf++ = LO_UINT16( statusRec->attrID );
        *pBuf++ = HI_UINT16( statusRec->attrID );
        *pBuf++ = statusRec->status;

        if ( statusRec->status == LTL_STATUS_SUCCESS ){
            *pBuf++ = statusRec->dataType;

            if ( statusRec->data != NULL ){
                // Copy attribute data to the buffer to be sent out
                pBuf = ltlSerializeData( statusRec->dataType, (void *)statusRec->data, pBuf );
            }
            else{
                dataLen = 0;
                // Read attribute data directly into the buffer to be sent out
                ltlReadAttrDataUsingCB( trunkID, nodeNO, statusRec->attrID, pBuf, &dataLen );
                pBuf += dataLen;
            }
        }
    } // for loop

    status = ltl_SendCommand(refer, trunkID, nodeNO, LTL_CMD_READ_ATTRIBUTES_RSP, FALSE,
              direction, disableDefaultRsp, 0, seqNum, buf, len );
    mo_free( buf );

    return ( status );
}


//ok
LStatus_t ltl_SendDefaultRspCmd( uint8_t *refer, uint16_t trunkID,uint8_t nodeNO,
                                 ltlDefaultRspCmd_t *defaultRspCmd, uint8_t direction,
                                 uint8_t disableDefaultRsp, uint16_t manuCode, uint8_t seqNum )
{
  uint8_t buf[2]; // Command ID and Status;

  // Load the buffer - serially
  buf[0] = defaultRspCmd->commandID;
  buf[1] = defaultRspCmd->statusCode;

  return ( ltl_SendCommand( refer, trunkID, nodeNO, LTL_CMD_DEFAULT_RSP, FALSE,
                            direction, disableDefaultRsp, manuCode, seqNum, buf, 2) );
}

/*********************************************************************
 * @fn      
 *
 * @brief   Builds a buffer from the attribute data
 *
 * @param   dataType - data types defined in LTL.h
 * @param   attrData - pointer to the attribute data
 * @param   buf - where to put the serialized data
 *
 * @return  pointer to end of destination buffer
 */
static uint8_t *ltlSerializeData( uint8_t dataType, void *attrData, uint8_t *buf )
{
    uint8_t *pStr;
    uint16_t len;

    switch ( dataType ){
        case LTL_DATATYPE_DATA8:
        case LTL_DATATYPE_BOOLEAN:
        case LTL_DATATYPE_BITMAP8:
        case LTL_DATATYPE_INT8:
        case LTL_DATATYPE_UINT8:
        case LTL_DATATYPE_ENUM8:
            *buf++ = *((uint8_t *)attrData);
            break;

        case LTL_DATATYPE_DATA16:
        case LTL_DATATYPE_BITMAP16:
        case LTL_DATATYPE_UINT16:
        case LTL_DATATYPE_INT16:
        case LTL_DATATYPE_ENUM16:
        //case LTL_DATATYPE_SEMI_PREC:
        case LTL_DATATYPE_TRUNK_ID:
        case LTL_DATATYPE_ATTR_ID:
            *buf++ = LO_UINT16( *((uint16_t *)attrData) );
            *buf++ = HI_UINT16( *((uint16_t *)attrData) );
            break;

        case LTL_DATATYPE_DATA32:
        case LTL_DATATYPE_BITMAP32:
        case LTL_DATATYPE_UINT32:
        case LTL_DATATYPE_INT32:
        case LTL_DATATYPE_SINGLE_PREC:
        //case LTL_DATATYPE_TOD:
        //case LTL_DATATYPE_DATE:
        //case LTL_DATATYPE_UTC:
        //case LTL_DATATYPE_BAC_OID:
            *buf++ = BREAK_UINT32( *((uint32_t *)attrData), 0 );
            *buf++ = BREAK_UINT32( *((uint32_t *)attrData), 1 );
            *buf++ = BREAK_UINT32( *((uint32_t *)attrData), 2 );
            *buf++ = BREAK_UINT32( *((uint32_t *)attrData), 3 );
            
            break;

        //case LTL_DATATYPE_IEEE_ADDR:
        //pStr = (uint8*)attrData;
        //buf = memcpy( buf, pStr, 8 );
        //break;

        case LTL_DATATYPE_CHAR_STR:
        case LTL_DATATYPE_OCTET_STR:
            pStr = (uint8_t *)attrData;
            len = *pStr;
            buf = memcpy( buf, pStr, len+1 ); // Including length field
            break;

        case LTL_DATATYPE_LONG_CHAR_STR:
        case LTL_DATATYPE_LONG_OCTET_STR:
            pStr = (uint8_t *)attrData;
            len = BUILD_UINT16( pStr[0], pStr[1] );
            buf = memcpy( buf, pStr, len+2 ); // Including length field
            break;

        case LTL_DATATYPE_128_BIT_SEC_KEY:
            pStr = (uint8_t *)attrData;
            buf = memcpy( buf, pStr, 8 );
            break;

        case LTL_DATATYPE_NO_DATA:
        case LTL_DATATYPE_UNKNOWN:
        // Fall through

        default:
            break;
  }

  return ( buf );
}


