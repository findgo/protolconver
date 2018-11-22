#ifndef __MT_MAC_H_
#define __MT_MAC_H_

#include "mt_npi.h"

/***************************************************************************************************
 * MAC COMMANDS
 ***************************************************************************************************/
/* SREQ/SRSP */
#define MT_MAC_RESET_REQ                     0x01
#define MT_MAC_INIT                          0x02
#define MT_MAC_START_REQ                     0x03
#define MT_MAC_SYNC_REQ                      0x04
#define MT_MAC_DATA_REQ                      0x05
#define MT_MAC_ASSOCIATE_REQ                 0x06
#define MT_MAC_DISASSOCIATE_REQ              0x07
#define MT_MAC_GET_REQ                       0x08
#define MT_MAC_SET_REQ                       0x09
#define MT_MAC_GTS_REQ                       0x0a
#define MT_MAC_RX_ENABLE_REQ                 0x0b
#define MT_MAC_SCAN_REQ                      0x0c
#define MT_MAC_POLL_REQ                      0x0d
#define MT_MAC_PURGE_REQ                     0x0e
#define MT_MAC_SET_RX_GAIN_REQ               0x0f

/* Security PIB SREQ/SRSP */
#define MT_MAC_SECURITY_GET_REQ              0x10
#define MT_MAC_SECURITY_SET_REQ              0x11

/* Enhanced Active Scan request */
#define MT_MAC_ENHANCED_ACTIVE_SCAN_REQ      0x12
#define MT_MAC_ENHANCED_ACTIVE_SCAN_RSP      0x13

/* Enhanced MAC interface SREQ/SRSP */
#define MT_MAC_SRC_MATCH_ENABLE              0x14
#define MT_MAC_SRC_MATCH_ADD_ENTRY           0x15
#define MT_MAC_SRC_MATCH_DELETE_ENTRY        0x16
#define MT_MAC_SRC_MATCH_ACK_ALL             0x17
#define MT_MAC_SRC_CHECK_ALL                 0x18

/* AREQ from Host */
#define MT_MAC_ASSOCIATE_RSP                 0x50
#define MT_MAC_ORPHAN_RSP                    0x51

/* AREQ to host */
#define MT_MAC_SYNC_LOSS_IND                 0x80
#define MT_MAC_ASSOCIATE_IND                 0x81
#define MT_MAC_ASSOCIATE_CNF                 0x82
#define MT_MAC_BEACON_NOTIFY_IND             0x83
#define MT_MAC_DATA_CNF                      0x84
#define MT_MAC_DATA_IND                      0x85
#define MT_MAC_DISASSOCIATE_IND              0x86
#define MT_MAC_DISASSOCIATE_CNF              0x87
#define MT_MAC_GTS_CNF                       0x88
#define MT_MAC_GTS_IND                       0x89
#define MT_MAC_ORPHAN_IND                    0x8a
#define MT_MAC_POLL_CNF                      0x8b
#define MT_MAC_SCAN_CNF                      0x8c
#define MT_MAC_COMM_STATUS_IND               0x8d
#define MT_MAC_START_CNF                     0x8e
#define MT_MAC_RX_ENABLE_CNF                 0x8f
#define MT_MAC_PURGE_CNF                     0x90
#define MT_MAC_POLL_IND                      0x91


// 本地宏
#define MTMAC_SendAsynchData(cmdID,dat,len)     NPISendAsynchData(MT_RPC_SYS_MAC, cmdID, dat, len)
#define MTMAC_SendSynchData(cmdID,dat,len)      NPISendSynchData(MT_RPC_SYS_MAC, cmdID, dat, len)

// sync command 



int mtmac_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtmac_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);


#endif

