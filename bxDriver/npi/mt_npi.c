
#include "mt_npi.h"
#include "common_type.h"
#include "mt_rpc.h"
#include "msglink.h"


// -- macros --
#define mtlogln(fmt, ...) log_debugln(fmt, ##__VA_ARGS__)

// State values for UART frame parsing
#define SOP_STATE      0x00
#define CMD_STATE1     0x01
#define CMD_STATE2     0x02
#define LEN_STATE      0x03
#define DATA_STATE     0x04
#define FCS_STATE      0x05

// -- Constants --
// Time out value for response from RNP
#define NPI_SYNC_RESPONSE_TIMEOUT     50 // 50 MS

// -- Typedefs --
typedef struct  {
	uint8_t state;
	uint8_t tempDataLen;
	uint8_t LEN_Token;
	uint16_t CMD_Token;
	uint8_t *data;
} npi_parseinfo_t;

typedef struct {
    uint8_t state;
    ctimer_t timer;
}npi_sycninfo_t;

// 同步请求信息头
typedef struct {
    uint16_t cmd;
    uint8_t len;
    uint8_t dat[];
}npi_syncreq_t;

extern int mt_SyncHandle(uint16_t commandID, uint8_t *data, uint8_t len);
extern int mt_AsncHandle(uint16_t commandID, uint8_t *data, uint8_t len);

// -- Local Variables --
static void npi_syncprocess(void);
static uint8_t npi_calcfcs(uint8_t len, uint16_t cmd, uint8_t *data);
static void npi_procframe( uint16_t cmd, uint8_t *pBuf, uint8_t length);

// for receive parse info
static npi_parseinfo_t npi_parseinfo;
static npi_sycninfo_t npi_syncinfo;
static msg_q_t npi_sync_q;


// -- Public functions --
void npiInit(void)
{
    SerialDrvInit(COM0, 115200, 8, DRV_PAR_NONE);
	npi_parseinfo.state = SOP_STATE;
    npi_syncinfo.state = 0;
}

/* build and send an NPI frame
 */
int npisendframe(uint16_t cmd, uint8_t *pData, uint8_t len)
{
    npi_syncreq_t *msg;
	uint8_t *pBuf;
    uint8_t frlen;
    
    if(len > MT_RPC_DATA_MAX)
        return NPI_LNX_FAILURE;

    len = (pData != NULL) ? len : 0;  
    frlen = MT_RPC_FRAME_OVHD + MT_RPC_PDU_HDR_SZ + len; // frame length

	msg = (npi_syncreq_t *) msgalloc(sizeof(npi_syncreq_t) + frlen);
	if (!msg) {
		return NPI_LNX_FAILURE;
	}

    pBuf = msg->dat;
    
	pBuf[0] = MT_RPC_UART_SOF;
	pBuf[1 + MT_RPC_POS_LEN] = len;
	pBuf[1 + MT_RPC_POS_CMD0] = HI_UINT16(cmd);
	pBuf[1 + MT_RPC_POS_CMD1] = LO_UINT16(cmd);
    if(pData){
	    memcpy(&pBuf[1 + MT_RPC_POS_DATA], pData, len);
    }
	pBuf[1 + len + MT_RPC_PDU_HDR_SZ] = npi_calcfcs(len, cmd, pData);
    
    if(cmd & MT_RPC_CMD_SREQ) {
        // wait for sync response
        msg->cmd = cmd;
        msg->len = frlen;
        msgQput(&npi_sync_q, msg);
    }
    else{
        npi_write(pBuf, frlen); // asnc req send soon
        msgdealloc(pBuf);
    }
    
	return NPI_LNX_SUCCESS;
}


void npiTask(void)
{
	uint8_t ch;
	int  bytesInRxBuffer;

	while (npi_readbufLen())
	{
	    npi_read(&ch, 1);
        
        switch (npi_parseinfo.state) {
		case SOP_STATE:
			if (ch == MT_RPC_UART_SOF)
				npi_parseinfo.state = LEN_STATE;
			break;

		case LEN_STATE:
			npi_parseinfo.LEN_Token = ch;
			npi_parseinfo.tempDataLen = 0;
        
            if( npi_parseinfo.LEN_Token > 0 ){
                /* Allocate memory for the data */
    			npi_parseinfo.data = (uint8_t *)mo_malloc(npi_parseinfo.LEN_Token );
                npi_parseinfo.state = (  npi_parseinfo.data == NULL) ? SOP_STATE : CMD_STATE1;
            }
            else{ 
                // 无需分配数据域
                npi_parseinfo.data = NULL;// make sure it NULL
			    npi_parseinfo.state = CMD_STATE1;
            }
			break;

		case CMD_STATE1:
			npi_parseinfo.CMD_Token = ch << 8;
			npi_parseinfo.state = CMD_STATE2;
			break;

		case CMD_STATE2:
			npi_parseinfo.CMD_Token |= ch;
			/* If there is no data, skip to FCS state */
			npi_parseinfo.state = (npi_parseinfo.LEN_Token > 0) ? DATA_STATE : FCS_STATE;
			break;

		case DATA_STATE:
			/* Fill in the buffer the first byte of the data */
			npi_parseinfo.data[npi_parseinfo.tempDataLen++] = ch;

			/* Check number of bytes left in the Rx buffer */
			bytesInRxBuffer = npi_readbufLen();

			/* If the remainder of the data is there, read them all, otherwise, just read enough */
            if(bytesInRxBuffer != 0){
    			if (bytesInRxBuffer <= npi_parseinfo.LEN_Token - npi_parseinfo.tempDataLen){
                    npi_read(&npi_parseinfo.data[npi_parseinfo.tempDataLen],bytesInRxBuffer);
    				npi_parseinfo.tempDataLen += (uint8_t) bytesInRxBuffer;
    			}
    			else{
    				npi_read(&npi_parseinfo.data[npi_parseinfo.tempDataLen],npi_parseinfo.LEN_Token - npi_parseinfo.tempDataLen);
    				npi_parseinfo.tempDataLen += (npi_parseinfo.LEN_Token - npi_parseinfo.tempDataLen);
    			}
            }
            
			/* If number of bytes read is equal to data length, time to move on to FCS */
			if ( npi_parseinfo.tempDataLen == npi_parseinfo.LEN_Token )
				npi_parseinfo.state = FCS_STATE;

			break;

		case FCS_STATE:
			/* Make sure it's correct */
			if ((npi_calcfcs(npi_parseinfo.LEN_Token,npi_parseinfo.CMD_Token, npi_parseinfo.data) == ch)) {
				npi_procframe(npi_parseinfo.CMD_Token, npi_parseinfo.data, npi_parseinfo.LEN_Token);
			}
            /* deallocate the msg */
            if(npi_parseinfo.data)
                mo_free ( (uint8_t *)npi_parseinfo.data );
			/* Reset the state, send or discard the buffers at this point */
			npi_parseinfo.state = SOP_STATE;
			break;

		default:  
			npi_parseinfo.state = SOP_STATE;
			break;
		}
	}
    npi_syncprocess();
}


static void npi_syncprocess(void)
{
    npi_syncreq_t *msg;
    
    switch(npi_syncinfo.state){
    case 0:
        // any request on the list?
        msg = (npi_syncreq_t *)msgQpeek(&npi_sync_q);
        if(msg){
            npi_write(msg->dat, msg->len);
            ctimerStart(npi_syncinfo.timer);
            npi_syncinfo.state = 1;
        }
        break;
        
    case 1: // wait for response
        if(ctimerExpired(npi_syncinfo.timer, NPI_SYNC_RESPONSE_TIMEOUT)){
            ( void )msgQpop(&npi_sync_q);
            npi_syncinfo.state  = 0;
        }
    
    }
}
/* Process received Pdu frame */
static void npi_procframe( uint16_t cmd, uint8_t *pBuf, uint8_t length)
{
    npi_syncreq_t *msg;
    uint16_t commandId;

    
    commandId = cmd & MT_RPC_SUBSYSTEM_MASK;
    mtlogln("cmd: 0x%04x,id: 0x%04x",cmd, commandId);
	if ( (cmd & MT_RPC_CMD_TYPE_MASK) == MT_RPC_CMD_SRSP) { // ((cmd & MT_RPC_SUBSYSTEM_MASK) == MT_RPC_SYS_BOOT)
		// process synchronous response
        if((msg = msgQpeek(&npi_sync_q)) == NULL)
            return;
        
        if(commandId  == (msg->cmd & MT_RPC_SUBSYSTEM_MASK)){
            msgdealloc(msgQpop(&npi_sync_q));
            npi_syncinfo.state  = 0;
           
            (void)mt_SyncHandle(commandId, pBuf , length);
        }
	}
	else {
		// process an asynchronous message
        (void)mt_AsncHandle(commandId, pBuf , length);
	}
}

/* Calculate NPI frame FCS */
static uint8_t npi_calcfcs( uint8_t len, uint16_t cmd, uint8_t *data_ptr )
{
	uint8_t i;
	uint8_t xorResult = len ^ HI_UINT16(cmd) ^ LO_UINT16(cmd);

	for ( i = 0; i < len; i++, data_ptr++ )
		xorResult = xorResult ^ *data_ptr;

	return ( xorResult );
}


