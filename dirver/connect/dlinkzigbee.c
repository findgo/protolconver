

#include "dlinkzigbee.h"

typedef struct dl_config_s{
    uint16_t shortaddress;
    uint16_t pandid;
    uint32_t bandrate;  
    uint16_t channel;
    uint8_t ttlpin4;
    uint8_t ttlpin5;
}dl_config_t;

static uint8_t dl_buf[DL_PACKET_BUFF_MAX];
//static uint8_t dl_len;
static dl_config_t dl_conf;

static void dl_info(uint8_t *dat,uint8_t dat_len);
static void dl_lchtime(uint16_t src_addr,uint8_t *dat,uint8_t dat_len);


/* check escape (port address and data )
 * return offset length
*/
#define DL_ESCAPE_CHAR1  0xff  // ff -> fe fd
#define DL_ESCAPE_CHAR2  0xfe  // fe -> fe fc
#define DL_ESCAPE_ASSIT_CHAR1  0xfd
#define DL_ESCAPE_ASSIT_CHAR2  0xfc
/*
    return total lengh
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
/*

    return total lengh
*/
static uint8_t __dl_escape_check_buff(uint8_t *buf,uint8_t *chk_buf,uint8_t buf_len)
{
    uint8_t i;
    uint8_t len;
    
    for(i = 0,len = 0;i < buf_len; i++){
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

const uint32_t band_tal[] = {2400,4800,9600,1440,19200,28800,38400,57600,115200,230400,125000,250000,500000};
/*index to bandrate*/
static uint32_t __band_parallel_table(uint8_t index)
{
    return band_tal[index];   
}
void dlink_request(uint8_t src_port,uint8_t dst_port,uint16_t dst_address,uint8_t *buf, uint8_t buf_len)
{
    uint8_t dl_len = 0;
    
    dl_buf[DL_PACKET_HEAD_OFS] = DL_PACKET_HEAD;
    dl_buf[DL_PACKET_LENGH_OFS] = buf_len + 4;
    dl_len += 2;

    dl_len += __dl_escape_check(&dl_buf[dl_len],src_port);
    dl_len += __dl_escape_check(&dl_buf[dl_len],dst_port);
//    dl_buf[dl_len++] = src_port;
//    dl_buf[dl_len++] = dst_port;

    dl_len += __dl_escape_check(&dl_buf[dl_len],dst_address & 0xff);
    dl_len += __dl_escape_check(&dl_buf[dl_len],(dst_address >> 8) & 0xff);
//    dl_buf[dl_len++] = dst_address & 0xff;
//    dl_buf[dl_len++] = (dst_address >> 8) & 0xff;
    
    dl_len += __dl_escape_check_buff(&dl_buf[dl_len],buf,buf_len);

    dl_buf[dl_len++] = DL_PACKET_TAIL;

    DL_SEND(dl_buf,dl_len);
}

/* request discover then light dst led 
 * 红灯led点亮时间 = time * 100ms
 */
void dlink_discover(uint16_t dst_address,uint8_t time)
{
    uint8_t tm = time;

    dlink_request_info(DL_PORT_DISCOVER,dst_address,&tm,1);
}

void dlink_rd_basic_info(uint8_t cmd)
{
    uint8_t scmd = cmd;

    dlink_request_info(DL_PORT_CONFIGURE,DL_LOCAL_ADD,&scmd,1);
}
void dlink_wr_basic_info(uint8_t cmd,uint16_t new_value)
{
    uint8_t tempbuf[3];
    
    if(cmd == DL_CONFIGURE_CMD_WR_ADD){
        if(new_value == DL_LOCAL_ADD || new_value == DL_BROADCAST_ADD)
            return;
        tempbuf[0] = DL_CONFIGURE_CMD_WR_ADD;
        tempbuf[1] = new_value & 0xff;
        tempbuf[2] = (new_value >> 8) & 0xff;
        dlink_request_info(DL_PORT_CONFIGURE,DL_LOCAL_ADD,tempbuf,3);
    }
    else if(cmd == DL_CONFIGURE_CMD_RD_PANID){
        if(new_value == DL_PAIND_INVALID)
            return;
        tempbuf[0] = DL_CONFIGURE_CMD_WR_PANID;
        tempbuf[1] = new_value & 0xff;
        tempbuf[2] = (new_value >> 8) & 0xff;
        dlink_request_info(DL_PORT_CONFIGURE,DL_LOCAL_ADD,tempbuf,3);
    }
    else if(cmd == DL_CONFIGURE_CMD_WR_CHANNEL){
        if((new_value < DL_CHANNEL_MIN) || (new_value > DL_CHANNEL_MAX))
            return;
        tempbuf[0] = DL_CONFIGURE_CMD_WR_CHANNEL;
        tempbuf[1] = new_value & 0xff;
        dlink_request_info(DL_PORT_CONFIGURE,DL_LOCAL_ADD,tempbuf,2);
    }
    else if(cmd == DL_CONFIGURE_CMD_WR_BAND){
        if(new_value > DL_BANDRATE_MAX)
            return;
        tempbuf[0] = DL_CONFIGURE_CMD_WR_BAND;
        tempbuf[1] = new_value & 0xff;
        dlink_request_info(DL_PORT_CONFIGURE,DL_LOCAL_ADD,tempbuf,2);
    }
    else if(cmd == DL_CONFIGURE_UPDATE_INFO_RESET){        
        tempbuf[0] = DL_CONFIGURE_UPDATE_INFO_RESET;
        dlink_request_info(DL_PORT_CONFIGURE,DL_LOCAL_ADD,tempbuf,1);
    }
}

void dlink_rssi(uint16_t dst_address,uint16_t collect_address)
{
    uint8_t tempbuf[2];

    if(collect_address != DL_LOCAL_ADD && collect_address != DL_BROADCAST_ADD){
        tempbuf[0] = collect_address & 0xff;
        tempbuf[1] = (collect_address >> 8) & 0xff;
        dlink_request_info(DL_PORT_RSSI,dst_address,tempbuf,2);
    }
}

void dlink_ttlpin4(uint16_t dst_address,uint8_t cmd)
{
    uint8_t scmd = cmd;

    dlink_request_info(DL_PORT_TTLPIN4,dst_address,&scmd,1);
}
void dlink_ttlpin5(uint16_t dst_address,uint8_t cmd)
{
    uint8_t scmd = cmd;

    dlink_request_info(DL_PORT_TTLPIN5,dst_address,&scmd,1);
}

static void dl_parse(uint8_t src_port,uint8_t dst_port,uint16_t src_addr,uint8_t *dat,uint8_t dat_len)
{    
    // only for info port message 
    if(DL_PORT_INFO == dst_port){
        switch (src_port){        
        // receive info from local zigbee device
        case DL_PORT_CONFIGURE:
            switch(dat[0]){
            case DL_CONFIGURE_RD_RSP_ADD:
                dl_conf.shortaddress = (dat[2] << 8) | dat[1];
                break;
            case DL_CONFIGURE_RD_RSP_PANID:
                dl_conf.pandid = (dat[2] << 8) | dat[1];
                break;
            case DL_CONFIGURE_RD_RSP_CHANNEL:
                dl_conf.channel = dat[1];
                break;
            case DL_CONFIGURE_RD_RSP_BAND:
                dl_conf.bandrate = __band_parallel_table(dat[1]);
                break;
            
            case DL_CONFIGURE_WR_RSP_SUCCESS:
                break;
            case DL_CONFIGURE_WR_RSP_NO_REMOTE_ACESS:
                break;
            case DL_CONFIGURE_WR_RSP_CMD_ERR:
                break;
            case DL_CONFIGURE_WR_RSP_LENGTH_ERR:
                break;            
            case DL_CONFIGURE_WR_RSP_VALUE_UNUSEALBE:
                break; 
            }
            break;
        case DL_PORT_ERR_REPORT:
            if(dat[0] == 0xe0){// must be 0x0e
                // dat[1]： Illegal port use by user
            }
            break;
        case DL_PORT_RSSI:
            //targetaddr = dat[1] << 8 | dat[0];
            //int8_t rssi_val = dat[2];
            //if(rssi_val == -1)
                //target not receive request
            break;       
        case DL_PORT_TTLPIN4:
            dl_conf.ttlpin4 = dat[0] == 0x00 ? 0x00 : 0x01;
            break;
        case DL_PORT_TTLPIN5:
            dl_conf.ttlpin5 = dat[0] == 0x00 ? 0x00 : 0x01;
            break;
        case DL_PORT_DISCOVER:
            break;
            
        // receive info from remote address  
        case DL_PORT_INFO: 
            dl_info(dat,dat_len);
 
        default: break;
        }
    }
    else if(DL_PORT_PASSTHROUGH == dst_port && src_port == DL_PORT_PASSTHROUGH){
        dl_lchtime(src_addr,dat,dat_len);
    }
    // other ignore ,and it may be error message
}

static void dl_info(uint8_t *dat,uint8_t dat_len)
{

}

static void dl_lchtime(uint16_t src_addr,uint8_t *dat,uint8_t dat_len)
{
    dlink_request_passthrough(src_addr, dat, dat_len);  
}

#define DL_FSM_HEAD  0
#define DL_FSM_LENGH 1
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
                    dl_packetbuf[3] << 8 | dl_packetbuf[2],
                    &dl_packetbuf[4],dl_packetlen - 4);
            }
            dl_fsm_state = DL_FSM_HEAD;
        default:          
            dl_fsm_state = DL_FSM_HEAD;
            break;
        }
    }
}


void dlink_init(void)
{
    // read zigbee device info
    dlink_discover(DL_LOCAL_ADD, 30);
    dlink_rd_basic_info(DL_CONFIGURE_CMD_RD_ADD);  
    dlink_rd_basic_info(DL_CONFIGURE_CMD_RD_PANID);
    dlink_rd_basic_info(DL_CONFIGURE_CMD_RD_CHANNEL);
    dl_conf.bandrate = 115200;
//    dlink_rd_basic_info(DL_CONFIGURE_CMD_RD_BAND);
}

