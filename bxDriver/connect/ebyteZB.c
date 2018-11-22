
#include "ebyteZB.h"
#include "flow_def.h"
#include "message.h"
#include "nwk.h"

#define EBYTE_UART_FORMAT_ERR       0xf7ff    // f7 ff 串口访问时格式错误 
#define EBYTE_COOR_SETUP            0xffff   // ff ff 协调器设备建立网络提示
#define EBYTE_JOININ_NET            0xffaa   // ff aa 设备加入网络会提示信息
#define EBYTE_NO_NET                0xff00   // ff 00 模块设备无网络或失去网络会提示信息

#define EBYTE_RESPONSE_TIMEOUT    500 // 回复超时设定

#define EBYTE_MSG_BOX_CAP_MAX      8

typedef struct {
    uint8_t headcmd;
    uint8_t cmd;
    uint16_t len;
    uint8_t dat[];
}ebytereq_t;

static void eb_processSequeue(void);
static void eb_CfgHandle(uint8_t *dat, uint8_t len);
static void eb_specificHandle(uint16_t speVal);

static ebyte_info_t ebyteinfo;

static msgboxhandle_t ebytemsghandle;
static msgboxstatic_t ebytemsgbuf;

static const ebyteNvPara_t ebytedefaltinfo = {
    EBYTE_NODE_DEVTYPE,
    EBYTE_CHANNEL_11,
    0xffff,

    EBYTE_BAND_115200,
    0x01,
    EBYTE_TXPOWER_4,
    30,

    0x77,0x77,0x77,0x2E,0x6C,0x63,0x68,0x74,0x69,0x6D,0x65,0x2E,0x63,0x6F,0x6D,0x2E // www.lchtime.com.
};


void ebyteZBInit(void)
{
    ebytemsghandle = msgBoxAssign(&ebytemsgbuf, EBYTE_MSG_BOX_CAP_MAX);
    SerialDrvInit(COM0, 115200, 8, DRV_PAR_NONE);
        
    memset(&ebyteinfo, 0, sizeof(ebyteinfo));
    ebyteinfo.nwk_state = EBYTE_NWK_STATE_OFF;
    ebyteinfo.valid = FALSE;
    ebyte_reset();
    mo_logln(DEBUG, "ebyte reset!");
}


uint8_t ebyte_NetAvailable(void)
{
    return (ebyteinfo.nwk_state == EBYTE_NWK_STATE_AVAILABLE && ebyteinfo.valid == TRUE);
}

uint16_t ebyte_nwkAddr(void)
{
   return ebyteinfo.nwkaddr;
}


// 返回读回复除去帧头的数据长度
static uint8_t ebyte_rdrsplen(uint8_t cmd)
{
    switch(cmd){
    case EBYTE_CMD_DEVTYPE:       
    case EBYTE_CMD_NWKSTATE:
    case EBYTE_CMD_GROUP:    
    case EBYTE_CMD_CHANNEL:          
    case EBYTE_CMD_TXPOWER:            
    case EBYTE_CMD_UARTBAND:           
    case EBYTE_CMD_SLEEPTIME:
    case EBYTE_CMD_DATA_SAVETIME:    
        return 1;
    case EBYTE_CMD_PAINID:
    case EBYTE_CMD_NWKADDR:
    case EBYTE_CMD_COOR_NWKADDR:
    case EBYTE_CMD_NWKADDR_ONNET:
        return 2;
    case EBYTE_CMD_NWKKEY:
        return EBYTE_NWKKEY_LEN;
    case EBYTE_CMD_MACADDR:
    case EBYTE_CMD_COOR_MACADDR:       
        return EBYTE_MAC_LEN;
    case EBYTE_CMD_ALLINFO:
        return EBYTE_INFO_LEN;
    case EBYTE_CMD_GPIO:            
    case EBYTE_CMD_GPIO_LEVEL: 
        return 4;
    case EBYTE_CMD_PWM:
        return 15;
    case EBYTE_CMD_ADC:
        return 5;
    default:
        return 0;
    }
}

// 返回写回复除去帧头的数据长度
static uint8_t ebyte_wrrsplen(uint8_t cmd)
{
    switch(cmd){
    case EBYTE_CMD_DEVTYPE:       
    case EBYTE_CMD_NWKSTATE:
    case EBYTE_CMD_GROUP:    
    case EBYTE_CMD_CHANNEL:          
    case EBYTE_CMD_TXPOWER:            
    case EBYTE_CMD_UARTBAND:           
    case EBYTE_CMD_SLEEPTIME:
    case EBYTE_CMD_DATA_SAVETIME:      
    case EBYTE_CMD_PAINID:
    case EBYTE_CMD_NWKADDR:
    case EBYTE_CMD_COOR_NWKADDR:
    case EBYTE_CMD_NWKADDR_ONNET:      
    case EBYTE_CMD_NWKKEY:
    case EBYTE_CMD_MACADDR:
    case EBYTE_CMD_COOR_MACADDR:                         
    case EBYTE_CMD_ALLINFO:
    case EBYTE_CMD_RESET:     
    case EBYTE_CMD_RESET_FACTORY:  
        return 1;
    case EBYTE_CMD_GPIO:            
    case EBYTE_CMD_GPIO_LEVEL: 
    case EBYTE_CMD_PWM:
        return 3;
    //case EBYTE_CMD_ADC: not support
    default:
        return 0;
    }
}
uint8_t ebyte_OSPFreq(uint16_t dstaddr, uint8_t *dat, uint8_t dat_len)
{
    uint8_t len = 0;
    uint8_t *pbuf;
    uint16_t tmpxor;

//    if(!ebyte_NetAvailable())
//        return FALSE;
    
    if(dat_len > EBYTE_PACKET_DATA_LEN - 4)
        return FALSE;

    pbuf = (uint8_t *)mo_malloc( 2 + 4 + (FLOW_SF_HEAD_LEN + FLOW_F_DATALEN_LEN + FLOW_F_XOR_LEN) + dat_len); 
    if(pbuf == NULL)
        return FALSE;
    
    pbuf[EBYTE_PACKET_HEAD_OFS] = EBYTE_PACKET_HEAD_CMD_DATA;
    pbuf[EBYTE_PACKET_LENGH_OFS] = dat_len + 4 + FLOW_SF_HEAD_LEN + FLOW_F_DATALEN_LEN + FLOW_F_XOR_LEN;
    len += 2;
    pbuf[len++] = EBYTE_UNICAST;
    pbuf[len++] = EBYTE_UNICAST_MODE_OSPF;
    pbuf[len++] = HI_UINT16(dstaddr);
    pbuf[len++] = LO_UINT16(dstaddr);

    // data 域  
    // for ospf flow解析
    pbuf[len++] = FLOW_SF_PREAMBLE;
    pbuf[len++] = dat_len;
    memcpy(&pbuf[len], dat, dat_len);
    len += dat_len;
    tmpxor = mXOR(dat, dat_len);
    pbuf[len++] = tmpxor;
    
    EBYTE_SEND(pbuf, len);

    mo_free(pbuf);
    
    return TRUE;
}

/*********************************************************************
 * @brief  send a request 
 *
 * @param   Headcmd - head command 
 * @param   dat - in -- pointer to buff you want to send
 * @param   dat_len  -- buff length
 *
 * @return  TRUE : success FALSE: failed
 */
uint8_t ebyte_cfgReq(uint8_t Headcmd, uint8_t cmd, uint8_t *premain, uint8_t reaminlen)
{
    ebytereq_t *msg;
    uint8_t *pbuf;
    uint8_t size;
    
    if(reaminlen > EBYTE_PACKET_DATA_LEN)
        return FALSE;

    reaminlen = (premain == NULL) ? 0 : reaminlen;
    size = 1 + 1 + 1 + reaminlen + 1;// headcmd + datalength + cmd + reaminlen + tail
    msg = (ebytereq_t *)msgalloc( sizeof(ebytereq_t) + size);
    if(msg == NULL)
        return FALSE;

    msg->headcmd = Headcmd; //save head command
    msg->cmd = cmd;  // save rd wr cmd
    msg->len = size;
    pbuf = msg->dat;

    // data to send
    *pbuf++ = Headcmd;
    *pbuf++ = reaminlen + 1;
    *pbuf++ = cmd;
    if(premain)
        memcpy(pbuf, premain, reaminlen);
    pbuf += reaminlen;
    *pbuf = EBYTE_PACKET_TAIL;

    if(msgBoxpost(ebytemsghandle, msg) < 0){
        msgdealloc(msg);
        return FALSE;
    }
    
    return TRUE;
}
#if 0
uint8_t ebyte_rdperipheral( uint8_t cmd, uint16_t nwkaddr, uint8_t gpio_pwm_adc)
{
    uint8_t tmpbuf[3];

    tmpbuf[0] = HI_UINT16(nwkaddr);
    tmpbuf[1] = LO_UINT16(nwkaddr);
    tmpbuf[2] = gpio_pwm_adc;

    return ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_RD, cmd, tmpbuf, 3);
}
#endif

uint8_t ebyte_wrGenericsingle(uint8_t cmd, uint8_t val)
{    
    return ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, cmd, &val, 1);
}

#if 0
uint8_t ebyte_wrPanid(uint16_t panid)
{
    uint8_t tmpbuf[2];
    
    tmpbuf[0] = HI_UINT16(panid);
    tmpbuf[1] = LO_UINT16(panid);
    
    return ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, EBYTE_CMD_PAINID, tmpbuf, 3);
}

uint8_t ebyte_wrGpio(uint8_t cmd, uint16_t nwkaddr, uint8_t gpio, uint8_t val)
{
    uint8_t tmpbuf[4];
    
    tmpbuf[0] = HI_UINT16(nwkaddr);
    tmpbuf[1] = LO_UINT16(nwkaddr);
    tmpbuf[2] = gpio;
    tmpbuf[3] = val;
    
    return ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, cmd, tmpbuf, 4);
}
#endif

uint8_t ebyte_wrAllInfo(ebyteNvPara_t *cfg)
{
    uint8_t *buf;
    uint8_t *pbuf;
    
    pbuf = buf = (uint8_t *)mo_malloc(EBYTE_INFO_LEN);
    if(buf == NULL)
        return FALSE;
    
    *pbuf++ = cfg->devType;
    *pbuf++ = 0xff;      // nwk_state;
    *pbuf++ = HI_UINT16( cfg->panid );
    *pbuf++ = LO_UINT16( cfg->panid );

    memcpy(pbuf, cfg->nwkkey, EBYTE_NWKKEY_LEN);
    pbuf += EBYTE_NWKKEY_LEN;
    memset(pbuf, 0xff, sizeof(uint16_t) * 2 + EBYTE_MAC_LEN * 2 ); // nwk addr + macaddr + coornwkaddr + coormac addr
    pbuf += sizeof(uint16_t) * 2 + EBYTE_MAC_LEN * 2;
    *pbuf++ = cfg->group;
    *pbuf++ = cfg->channel;
    *pbuf++ = cfg->txpower;
    *pbuf++ = 0x09;
    *pbuf = cfg->node_time;

    ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, EBYTE_CMD_ALLINFO, buf, EBYTE_INFO_LEN);

    mo_free(buf);

    return TRUE;
}

#define EBYTE_FSM_HEAD              0
#define EBYTE_FSM_SPECIAL           1
#define EBYTE_FSM_CFG_TOKEN         2
#define EBYTE_FSM_OSPF_LEN          3
#define EBYTE_FSM_OSPF_TOKEN        4
#define EBYTE_FSM_OSPF_XOR          5

static uint8_t *eb_pbuf = NULL;
static uint8_t eb_packetlen;
static uint8_t eb_packetbytes;
static uint16_t eb_specific;
static uint8_t eb_fsm_state = EBYTE_FSM_HEAD;

static uint8_t eb_reqHeadcmd = 0xff;
static uint8_t eb_reqcmd = 0xff;
static uint8_t eb_seqstate = FALSE;

static void __ebyte_InReqRspCheck(uint8_t ch)
{
    if(eb_seqstate){
        // headcmd +  get read rsp or write rsp length 
        eb_packetlen = 1 +   ((ch == EBYTE_PACKET_HEAD_CMD_RD_RSP ) ? ebyte_rdrsplen(eb_reqcmd) : ebyte_wrrsplen(eb_reqcmd));    
        eb_packetbytes = 0;
        eb_pbuf = (uint8_t *)mo_malloc(eb_packetlen);
        if(eb_pbuf){
            eb_pbuf[eb_packetbytes++] = ch; // save head?
            eb_fsm_state =  EBYTE_FSM_CFG_TOKEN;
        }
        else{
            eb_fsm_state =  EBYTE_FSM_HEAD;
        }
    }
    else {
        eb_fsm_state = EBYTE_FSM_HEAD;
    }
}


void ebyteZBTask(void)
{
    uint8_t ch;
    uint16_t bytesInRxBuffer;

    while(EBYTE_RCVBUFLEN())
    {        
        EBYTE_RCV(&ch, 1); //read one byte
        
        switch (eb_fsm_state){
        case EBYTE_FSM_HEAD:
            if(ch == FLOW_SF_PREAMBLE){ // OSPF head
                eb_fsm_state = EBYTE_FSM_OSPF_LEN;
            }
            else if ( ch == EBYTE_PACKET_HEAD_CMD_RD_RSP || ch == EBYTE_PACKET_HEAD_CMD_WR_RSP ){ 
                __ebyte_InReqRspCheck(ch);
            }
            else if(ch == EBYTE_PACKET_HEAD_CMD_UART_ERR || ch == EBYTE_PACKET_HEAD_CMD_SPECIFIC){
                eb_specific =  ch;
                eb_fsm_state = EBYTE_FSM_SPECIAL;
            }
            else{
                mo_logln(DEBUG, "invalid state!");
                // do nothing
            }
            break;
            
        case EBYTE_FSM_SPECIAL:
            eb_specific <<= 8;
            eb_specific |=  ch;
            if(eb_specific ==  EBYTE_JOININ_NET|| eb_specific ==   EBYTE_NO_NET
                || eb_specific == EBYTE_COOR_SETUP || eb_specific ==   EBYTE_UART_FORMAT_ERR) {
                eb_specificHandle(eb_specific);
                eb_fsm_state = EBYTE_FSM_HEAD;
            } 
            else if(ch == FLOW_SF_PREAMBLE){
                eb_fsm_state = EBYTE_FSM_OSPF_LEN;
            }
            else if( ch == EBYTE_PACKET_HEAD_CMD_RD_RSP || ch == EBYTE_PACKET_HEAD_CMD_WR_RSP ){
                __ebyte_InReqRspCheck(ch);
            }
            else{
                eb_fsm_state = EBYTE_FSM_HEAD;
            }
            break;
            
        case EBYTE_FSM_CFG_TOKEN:           
            eb_pbuf[eb_packetbytes++] = ch;
            
            bytesInRxBuffer = EBYTE_RCVBUFLEN();
            /* If the remain of the data is there, read them all, otherwise, just read enough */
            
            if(bytesInRxBuffer != 0){
                if (bytesInRxBuffer <= ( eb_packetlen - eb_packetbytes )){
                    EBYTE_RCV(&eb_pbuf[eb_packetbytes], bytesInRxBuffer);
                    eb_packetbytes += bytesInRxBuffer;
                }
                else{
                    EBYTE_RCV(&eb_pbuf[eb_packetbytes], eb_packetlen - eb_packetbytes);
                    eb_packetbytes += eb_packetlen - eb_packetbytes;
                }
            }

            if(eb_packetbytes == eb_packetlen){
                mo_logln(DEBUG, "rsp handle!");
                eb_CfgHandle(eb_pbuf, eb_packetlen);
                eb_seqstate = FALSE;
                eb_fsm_state = EBYTE_FSM_HEAD;
                mo_free(eb_pbuf);
            }
            break;
            
        case EBYTE_FSM_OSPF_LEN:
            eb_packetlen = ch;
            eb_packetbytes = 0;

            eb_pbuf = (uint8_t *)msgalloc(eb_packetlen);
            if(eb_pbuf){
                eb_fsm_state = EBYTE_FSM_OSPF_TOKEN;
            }
            else{               
                eb_fsm_state = EBYTE_FSM_HEAD;
            }
            break;

        case EBYTE_FSM_OSPF_TOKEN:
            eb_pbuf[eb_packetbytes++] = ch;

            bytesInRxBuffer = EBYTE_RCVBUFLEN();
            /* If the remain of the data is there, read them all, otherwise, just read enough */
            
            if(bytesInRxBuffer != 0){
                if (bytesInRxBuffer <= ( eb_packetlen - eb_packetbytes )){
                    EBYTE_RCV (&eb_pbuf[eb_packetbytes], bytesInRxBuffer);
                    eb_packetbytes += bytesInRxBuffer;
                }
                else{
                    EBYTE_RCV (&eb_pbuf[eb_packetbytes], eb_packetlen - eb_packetbytes);
                    eb_packetlen += eb_packetlen - eb_packetbytes;
                }
            }

            if(eb_packetlen == eb_packetbytes){
                eb_fsm_state = EBYTE_FSM_OSPF_XOR;
            }                
            break;
            
        case EBYTE_FSM_OSPF_XOR:
            if(mXOR(eb_pbuf, eb_packetlen) == ch){
                mo_logln(DEBUG, "OSPF sendto nwk!");
                if(nwkmsgsend(eb_pbuf) < 0)
                    msgdealloc(eb_pbuf);
            }
            else{
                msgdealloc(eb_pbuf);
            }
            eb_fsm_state = EBYTE_FSM_HEAD;
            break;
            
        default:          
            eb_fsm_state = EBYTE_FSM_HEAD;
            break;
        }
    }  
    eb_processSequeue();
}

static void eb_processSequeue(void)
{
    ebytereq_t *msg;
    static ctimer_t ebtm;
    
    // in rsp sequeue
    if(eb_seqstate == TRUE){
        if(ctimerExpired(ebtm, EBYTE_RESPONSE_TIMEOUT)){
            eb_seqstate = FALSE;
            mo_logln(DEBUG, "ebyte rsp timeout!");
        }
        
        return;
    }

    if((msg = (ebytereq_t *)msgBoxaccept(ebytemsghandle)) == NULL)
        return;

    eb_reqHeadcmd = msg->headcmd;
    eb_reqcmd = msg->cmd;
    mo_logln(DEBUG, "ebyte request send! %x %x", eb_reqHeadcmd, eb_reqcmd);
    EBYTE_SEND(msg->dat,  msg->len);  
    msgdealloc(msg);
    eb_seqstate = TRUE;
    ctimerStart(ebtm);
}


static void eb_specificHandle(uint16_t speVal)
{
    mo_logln(DEBUG,"specific Handle value: 0x%x",speVal);
    if(speVal == EBYTE_COOR_SETUP){
        #if EBYTE_NODE_DEVTYPE == EBYTE_DEVTYPE_COOR
            ebyteinfo.nwk_state = EBYTE_NWK_STATE_AVAILABLE;      // ff aa 设备加入网络会提示信息
            // 协调器启动一个信息获取序列
            //ebyte_rdAllInfo();
        #endif
    }
#if EBYTE_NODE_DEVTYPE != EBYTE_DEVTYPE_COOR
    else if (speVal == EBYTE_JOININ_NET){ // ff aa 设备加入网络会提示信息
            ebyteinfo.nwk_state = EBYTE_NWK_STATE_AVAILABLE;      
            //启动一个获取信息序列
            ebyte_rdAllInfo();
    }
#endif
    else if(speVal == EBYTE_NO_NET) { // ff 00 模块设备无网络或失去网络会提示信息
        ebyteinfo.nwk_state = EBYTE_NWK_STATE_OFF;
        ebyteinfo.valid = FALSE;
    }
    else if(speVal == EBYTE_UART_FORMAT_ERR){ // f7 ff 串口访问时格式错误 
        // check you frame format
    }
}
static void eb_CfgHandle(uint8_t *dat, uint8_t len)
{
    uint8_t lens;
    // read rsp
    if(dat[0] == EBYTE_PACKET_HEAD_CMD_RD_RSP && eb_reqHeadcmd == EBYTE_PACKET_HEAD_CMD_RD){
        switch (eb_reqcmd){
        case EBYTE_CMD_ALLINFO:
            lens = 2;
            ebyteinfo.devType = dat[lens++];
            ebyteinfo.nwk_state = dat[lens++];
            ebyteinfo.panid = BUILD_UINT16(dat[lens + 1], dat[lens]);
            lens += 2;
            memcpy(ebyteinfo.nwkkey, &dat[lens], EBYTE_NWKKEY_LEN);
            lens += EBYTE_NWKKEY_LEN;

            ebyteinfo.nwkaddr = BUILD_UINT16(dat[lens + 1], dat[lens]);
            lens += 2;
            memcpy(ebyteinfo.macaddr, &dat[lens], EBYTE_MAC_LEN);
            lens += EBYTE_MAC_LEN;

            ebyteinfo.coor_nwkaddr = BUILD_UINT16(dat[lens + 1], dat[lens]);
            lens += 2;
            memcpy(ebyteinfo.coor_macaddr, &dat[lens], EBYTE_MAC_LEN);
            lens += EBYTE_MAC_LEN;

            ebyteinfo.group = dat[lens++];
            ebyteinfo.channel = dat[lens++];
            ebyteinfo.txpower = dat[lens++];
            ebyteinfo.band = dat[lens++];
            ebyteinfo.sleep_time = dat[lens++];
            ebyteinfo.valid = TRUE;

            break;
        case EBYTE_CMD_DATA_SAVETIME:
            ebyteinfo.data_savetime = dat[2];
            break;
        }

    }
    else{ // write rsp
        //忽略
    }
}

