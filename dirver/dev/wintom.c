

#include "wintom.h"

static uint8_t wt_txbuf[16];

void wintom_request(uint8_t cmdCode, uint8_t ofs1,uint8_t ofs2,
                        uint8_t * parabuf, uint8_t paralen)
{
    uint8_t i;
    uint8_t wt_len = 0;
    uint8_t checksum = 0;

    
    wt_txbuf[wt_len++] = WT_PACKET_HEAD_MSB;
    wt_txbuf[wt_len++] = WT_PACKET_HEAD_LSB;
    wt_txbuf[wt_len++] = 3 + (parabuf == NULL ? 0 : paralen);
    
    wt_txbuf[wt_len++] = cmdCode;
    checksum += cmdCode;
    wt_txbuf[wt_len++] = moto_no;
    checksum += moto_no;
    wt_txbuf[wt_len++] = devID;
    checksum += devID;
    
    if(parabuf != null){
        for(i = 0;i < paralen; i++){
            wt_txbuf[wt_len] = *parabuf;
            checksum += *parabuf;
            parabuf++;
            wt_len++;
        }
    }

    wt_txbuf[wt_len++] = checksum;

    WT_SEND(wt_txbuf,wt_len);
}

/*  pos 0x00 - 0xff
    停位点坐标 = 总行程/256 * 停位点 */
void wintom_runtoPos(uint8_t moto_no, uint8_t devID, uint8_t pos)
{
    uint8_t tpos = pos;

    wintom_request(WT_CMDCODE_GO_POS,moto_num, devID,&tpos,1);
}

void wintom_setAngle(uint8_t moto_no, uint8_t devID, uint8_t angle)
{
    uint8_t tangle = angle;

    tangle = angle > 180 ? 180 : angle;

    wintom_request(WT_CMDCODE_GO_POS,moto_num, devID,&tangle,1);
}
/*  special command */
void wintom_setDevID(uint8_t channel, uint8_t moto_no, uint8_t devID)
{
    uint8_t tdevID = devID;

    wintom_request(WT_CMDCODE_SET_DEVID,channel,moto_no,&tdevID,1);
}
void wintom_getSingleDevID(uint8_t channel)
{
    uint8_t wt_len = 0;
    uint8_t checksum = 0;

    wt_txbuf[wt_len++] = WT_PACKET_HEAD_MSB;
    wt_txbuf[wt_len++] = WT_PACKET_HEAD_LSB;
    wt_txbuf[wt_len++] = 2;
    
    wt_txbuf[wt_len++] = WT_CMDCODE_GET_DEVID;
    checksum += WT_CMDCODE_GET_DEVID;
    wt_txbuf[wt_len++] = channel;
    checksum += channel;

    wt_txbuf[wt_len++] = checksum;

    WT_SEND(wt_txbuf,wt_len);
}

//WT_CMDCODE_GET_POS
//WT_CMDCODE_GET_ANGLE
//WT_CMDCODE_GET_DEVID
//WT_CMDCODE_GET_STATUS