

#include "dlinkzigbee.h"

static uint8_t dl_buf[DL_PACKET_BUFF_MAX];
static dl_basicInfo_t dl_basicInfo;
static dl_apduParsepfn_t dl_info_cb = NULL;
static dl_apduParsepfn_t dl_passthrough_cb = NULL;

/*********************************************************************
* @brief  对chk_dat数据进行转义
*
* @param   buf  -- out -- pointer the buffer where to write
* @param   chk_dat  - check data
*
* @return  total lengh
*/
static uint8_t __dl_escape_check(uint8_t *buf,uint8_t chk_dat)
{
    if(chk_dat == DL_ESCAPE_CHAR1){
        buf[0]  = DL_ESCAPE_CHAR2;
        buf[1]  = DL_ESCAPE_ASSIT_CHAR1;
    
        return 2;
    }
    else if(chk_dat == DL_ESCAPE_CHAR2){
        buf[0] = DL_ESCAPE_CHAR2;
        buf[1] = DL_ESCAPE_ASSIT_CHAR2;
    
        return 2;
    }else{
        buf[0] = chk_dat;
        
        return 1;
    }
}
/*********************************************************************
* @brief  对chk buff数据进行转义
*
* @param   buf  -- out -- pointer the buffer where to write
* @param   chk_buf  - in --  pointer the buffer check
* @param   chkbuf_len  - check buffer length
*
* @return  total lengh
*/
static uint8_t __dl_escape_check_buff(uint8_t *buf, uint8_t *chk_buf, uint8_t chkbuf_len)
{
    uint8_t i;
    uint8_t len;
    
    for( i = 0, len = 0;i < chkbuf_len; i++ ){
        if(chk_buf[i] == DL_ESCAPE_CHAR1){
            buf[len++]  = DL_ESCAPE_CHAR2;
            buf[len++]  = DL_ESCAPE_ASSIT_CHAR1;
        }
        else if(chk_buf[i]  == DL_ESCAPE_CHAR2){
            buf[len++]  = DL_ESCAPE_CHAR2;
            buf[len++]  = DL_ESCAPE_ASSIT_CHAR2;
        }else{
            buf[len++]  = chk_buf[i];
        }    
    }

    return len;
}

const uint32_t band_tal[] = { 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400, 125000, 250000, 500000 };
/*********************************************************************
* @brief  index to bandrate
*
* @param   index  - index
*
* @return  real bandrate
*/
static uint32_t __band_parallel_table(uint8_t index)
{
    return band_tal[index];   
}

/*********************************************************************
 * @brief  send a request 
 *
 * @param   src_port - source port 
 * @param   dst_port - dest port
 * @param   dst_address  - dest addres you want to send
 * @param   dat - in -- pointer to buff you want to send
 * @param   dat_len  -- buff length
 *
 * @return  TRUE : success FALSE: failed
 */
uint8_t dlink_request(uint8_t src_port,uint8_t dst_port,uint16_t dst_address,uint8_t *dat, uint8_t dat_len)
{
    uint8_t dl_len = 0;

    // may be bug,because no contain escape?????
    if((dat_len + 4 + 2) > DL_PACKET_BUFF_MAX)
        return FALSE;
    
    dl_buf[DL_PACKET_HEAD_OFS] = DL_PACKET_HEAD;
    dl_buf[DL_PACKET_LENGH_OFS] = dat_len + 4; // 包长度并不受转义影响，转义增加的不计算在内.*/
    dl_len += 2;

    dl_len += __dl_escape_check(&dl_buf[dl_len],src_port);
    dl_len += __dl_escape_check(&dl_buf[dl_len],dst_port);

    dl_len += __dl_escape_check(&dl_buf[dl_len],dst_address & 0xff);
    dl_len += __dl_escape_check(&dl_buf[dl_len],(dst_address >> 8) & 0xff);
    
    dl_len += __dl_escape_check_buff(&dl_buf[dl_len], dat, dat_len);

    dl_buf[dl_len++] = DL_PACKET_TAIL;

    DL_SEND(dl_buf,dl_len);

    return TRUE;
}


 /*********************************************************************
 * @brief  request discover then light dst led 
 *
 * @param   dst_address  - dest addres you want to send
 * @param   time - led点亮时间 = time * 100ms
 *
 * @return  TRUE : success FALSE: failed
 */
uint8_t dlink_discover_request(uint16_t dst_address, uint8_t time)
{
    uint8_t tm = time;

    return dlink_info_request(DL_PORT_DISCOVER, dst_address, &tm, 1);
}
/*********************************************************************
* @brief  read  basic info
*
* @param   cmd - the info which you want to read
*
* @return  TRUE : success FALSE: failed
*/
uint8_t dlink_rd_local_basic_info(uint8_t cmd)
{
    uint8_t scmd = cmd;

    return dlink_info_request(DL_PORT_BASIC_INFO, DL_LOCAL_ADD, &scmd, 1);
}
/*********************************************************************
* @brief  write  basic info
*
* @param   cmd - the info which you want to write
* @param   new_value - new value
*
* @return  TRUE : success FALSE: failed
*/
uint8_t dlink_wr_local_basic_info(uint8_t cmd, uint16_t new_value)
{
    uint8_t len = 0;
    uint8_t tempbuf[3];
    
    switch (cmd){
    case DL_BASIC_INFO_CMD_WR_ADD:
        if(new_value == DL_LOCAL_ADD || new_value == DL_BROADCAST_ADD)
            return FALSE;
        
        tempbuf[0] = DL_BASIC_INFO_CMD_WR_ADD;
        tempbuf[1] = LO_UINT16(new_value);
        tempbuf[2] = HI_UINT16(new_value);
        len = 3;
        break;
        
    case DL_BASIC_INFO_CMD_RD_PANID:
        if(new_value == DL_PAIND_INVALID)
            return FALSE;
        
        tempbuf[0] = DL_BASIC_INFO_CMD_WR_PANID;
        tempbuf[1] = LO_UINT16(new_value);
        tempbuf[2] = HI_UINT16(new_value);
        len = 3;
        break;
        
    case DL_BASIC_INFO_CMD_WR_CHANNEL:
        if((new_value < DL_CHANNEL_MIN) || (new_value > DL_CHANNEL_MAX))
            return FALSE;
        
        tempbuf[0] = DL_BASIC_INFO_CMD_WR_CHANNEL;
        tempbuf[1] = LO_UINT16(new_value);
        len = 2;
        break;
        
    case DL_BASIC_INFO_CMD_WR_BAND:
        
        if(new_value > DL_BANDRATE_MAX)
            return FALSE;
        
        tempbuf[0] = DL_BASIC_INFO_CMD_WR_BAND;
        tempbuf[1] = LO_UINT16(new_value);
        len = 2;
        break;
        
    case DL_BASIC_INFO_UPDATE_INFO_RESET:
        
        tempbuf[0] = DL_BASIC_INFO_UPDATE_INFO_RESET;
        len = 1;
        break;
        
        default:
            return FALSE;
    }

    return dlink_info_request(DL_PORT_BASIC_INFO, DL_LOCAL_ADD, tempbuf, len);
}
/*********************************************************************
* @brief  request rssi
*
* @param   dst_address - dest address 
* @param   collect_address - collect address
*
* @return  TRUE : success FALSE: failed
*/
uint8_t dlink_rssi_request(uint16_t dst_address,uint16_t collect_address)
{
    uint8_t tempbuf[2];

    if(collect_address != DL_LOCAL_ADD && collect_address != DL_BROADCAST_ADD){
        tempbuf[0] = LO_UINT16( collect_address );
        tempbuf[1] = HI_UINT16( collect_address );
    
        return dlink_info_request(DL_PORT_RSSI, dst_address, tempbuf, 2);
    }

    return FALSE;
}
/*********************************************************************
* @brief  set ttl pin4 status
*
* @param   dst_address - dest address 
* @param   cmd - command
*
* @return  TRUE : success FALSE: failed
*/
uint8_t dlink_ttlpin4_requset(uint16_t dst_address,uint8_t cmd)
{
    uint8_t scmd = cmd;

    return dlink_info_request(DL_PORT_TTLPIN4, dst_address, &scmd, 1);
}
/*********************************************************************
* @brief  set ttl pin5 status
*
* @param   dst_address - dest address 
* @param   cmd - command
*
* @return  TRUE : success FALSE: failed
*/
uint8_t dlink_ttlpin5_request(uint16_t dst_address,uint8_t cmd)
{
    uint8_t scmd = cmd;

    return dlink_info_request(DL_PORT_TTLPIN5, dst_address, &scmd, 1);
}

/*********************************************************************
* @brief  报文解析
*
* @return pointer to basic info 
*/

dl_basicInfo_t *dlink_getbasicInfo(void)
{
    return &dl_basicInfo;
}
/*********************************************************************
* @brief  报文解析
*
* @param   src_port - source port 
* @param   dst_port - dest port 
* @param   src_addr - source address 
* @param   dat - pointer to the data
* @param   dat_len - data length
*
* @return 
*/
static void dl_parse(uint8_t src_port,uint8_t dst_port,uint16_t src_addr,uint8_t *dat,uint8_t dat_len)
{    
    // only for info port message 
    if(DL_PORT_INFO == dst_port){
        switch (src_port){        
        // receive info from local zigbee device
        case DL_PORT_BASIC_INFO:
            switch(dat[0]){
            case DL_BASIC_INFO_RD_RSP_ADD:
                dl_basicInfo.shortaddress =  BUILD_UINT16(dat[1], dat[2]);
                break;
            
            case DL_BASIC_INFO_RD_RSP_PANID:
                dl_basicInfo.pandid =  BUILD_UINT16(dat[1], dat[2]);
                break;
            
            case DL_BASIC_INFO_RD_RSP_CHANNEL:
                dl_basicInfo.channel = dat[1];
                break;
            
            case DL_BASIC_INFO_RD_RSP_BAND:
                dl_basicInfo.bandrate = __band_parallel_table(dat[1]);
                break;

            case DL_BASIC_INFO_WR_RSP_SUCCESS:
            case DL_BASIC_INFO_WR_RSP_NO_REMOTE_ACESS:
            case DL_BASIC_INFO_WR_RSP_CMD_ERR:
            case DL_BASIC_INFO_WR_RSP_LENGTH_ERR:
            case DL_BASIC_INFO_WR_RSP_VALUE_UNUSEALBE:
                break; 
            }
            break;
            
        case DL_PORT_ERR_REPORT:
            if(dat[0] == 0xe0){// must be 0xe0
                // dat[1]： Illegal port use by user
            }
            break;
            
        case DL_PORT_RSSI:
            //targetaddr =  BUILD_UINT16(dat[1], dat[2]);
            //int8_t rssi_val = dat[2];
            //if(rssi_val == -1)
                //target not receive request
            break;       
        case DL_PORT_TTLPIN4:
            dl_basicInfo.ttlpin4 = dat[0] == 0x00 ? 0x00 : 0x01;
            break;
        
        case DL_PORT_TTLPIN5:
            dl_basicInfo.ttlpin5 = dat[0] == 0x00 ? 0x00 : 0x01;
            break;
        // no response
        //case DL_PORT_DISCOVER:
        //    break;
            
        // receive info from remote address  
        case DL_PORT_INFO:
            if(dl_info_cb)
                dl_info_cb(src_addr,dat,dat_len);
 
        default: break;
        }
    }
    else if((DL_PORT_PASSTHROUGH == dst_port) && (DL_PORT_PASSTHROUGH == src_port) && dl_passthrough_cb){
        dl_passthrough_cb(src_addr, dat, dat_len);
    }
    // other ignore ,and it may be error message
}

#define DL_FSM_HEAD         0
#define DL_FSM_LENGH        1
#define DL_FSM_TOKEN        2
#define DL_FSM_ESCAPE       3
#define DL_FSM_TAIL         4

static uint8_t dl_packetlen;
static uint8_t dl_packetbuf[DL_PACKET_BUFF_MAX];
static uint8_t dl_packetbytes;
static uint8_t dl_fsm_state = DL_FSM_HEAD;

void dlink_period_task(void)
{
    uint8_t ch;

    while(DL_RCVBUFLEN())
    {
        DL_RCV(&ch, 1); //read one byte

        switch (dl_fsm_state){
        case DL_FSM_HEAD:
            if (ch == DL_PACKET_HEAD)
                dl_fsm_state = DL_FSM_LENGH;
            break;
          
        case DL_FSM_LENGH:
            // out of max packet lengh
            if(ch > DL_PACKET_BUFF_MAX)
                dl_fsm_state = DL_FSM_HEAD;
            
            dl_packetlen = ch;
            dl_packetbytes = 0;
            dl_fsm_state = DL_FSM_TOKEN;
            break;

        case DL_FSM_TOKEN:
            if(ch == DL_ESCAPE_CHAR2)
                dl_fsm_state = DL_FSM_ESCAPE;
            else{
                dl_packetbuf[dl_packetbytes] = ch;
                ++dl_packetbytes;
                if(dl_packetbytes >=  dl_packetlen)
                    dl_fsm_state = DL_FSM_TAIL;
            }
            break;
        case DL_FSM_ESCAPE:
            if(ch == DL_ESCAPE_ASSIT_CHAR1 || ch == DL_ESCAPE_ASSIT_CHAR2){
                dl_packetbuf[dl_packetbytes] = (ch == DL_ESCAPE_ASSIT_CHAR1) ? DL_ESCAPE_CHAR1 : DL_ESCAPE_CHAR2;               
                ++dl_packetbytes;
                if(dl_packetbytes >=  dl_packetlen)
                    dl_fsm_state = DL_FSM_TAIL;
                else
                    dl_fsm_state = DL_FSM_TOKEN;
            }
            else{//error
                dl_fsm_state = DL_FSM_HEAD;
            }
            break;
        case DL_FSM_TAIL:
            if(ch == DL_PACKET_TAIL){
                dl_parse(dl_packetbuf[0],dl_packetbuf[1],
                            BUILD_UINT16(dl_packetbuf[2], dl_packetbuf[3]),
                            &dl_packetbuf[4],dl_packetlen - 4);
            }
            dl_fsm_state = DL_FSM_HEAD;
        default:          
            dl_fsm_state = DL_FSM_HEAD;
            break;
        }
    }
}
/*********************************************************************
* @brief  注册APDU解析回调函数
*
* @param   info_cb - 消息回调
* @param   passthrough_cb - 透传回调
*
* @return 
*/
uint8_t dl_registerParseCallBack(dl_apduParsepfn_t info_cb, dl_apduParsepfn_t passthrough_cb)
{
    dl_info_cb = info_cb;
    dl_passthrough_cb = passthrough_cb;

    return TRUE;
}

void dlink_init(void)
{
    // read zigbee device info
    dlink_discover_request(DL_LOCAL_ADD, 30);
    dlink_rd_local_basic_info(DL_BASIC_INFO_CMD_RD_ADD);  
    dlink_rd_local_basic_info(DL_BASIC_INFO_CMD_RD_PANID);
    dlink_rd_local_basic_info(DL_BASIC_INFO_CMD_RD_CHANNEL);
    dl_basicInfo.bandrate = 115200;
//    dlink_rd_basic_info(DL_BASIC_INFO_CMD_RD_BAND);
}
