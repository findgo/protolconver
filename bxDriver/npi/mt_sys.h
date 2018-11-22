

#ifndef __MT_SYS_H__
#define __MT_SYS_H__

#include "mt_npi.h"

/***************************************************************************************************
 * SYS COMMANDS
 ***************************************************************************************************/

/* AREQ from host */
#define MT_SYS_RESET_REQ                     0x00

/* SREQ/SRSP */
#define MT_SYS_PING                          0x01
#define MT_SYS_VERSION                       0x02
#define MT_SYS_SET_EXTADDR                   0x03
#define MT_SYS_GET_EXTADDR                   0x04
#define MT_SYS_RAM_READ                      0x05
#define MT_SYS_RAM_WRITE                     0x06
#define MT_SYS_OSAL_NV_ITEM_INIT             0x07
#define MT_SYS_OSAL_NV_READ                  0x08
#define MT_SYS_OSAL_NV_WRITE                 0x09
#define MT_SYS_OSAL_START_TIMER              0x0A
#define MT_SYS_OSAL_STOP_TIMER               0x0B
#define MT_SYS_RANDOM                        0x0C
#define MT_SYS_ADC_READ                      0x0D
#define MT_SYS_GPIO                          0x0E
#define MT_SYS_STACK_TUNE                    0x0F
#define MT_SYS_SET_TIME                      0x10
#define MT_SYS_GET_TIME                      0x11
#define MT_SYS_OSAL_NV_DELETE                0x12
#define MT_SYS_OSAL_NV_LENGTH                0x13
#define MT_SYS_SET_TX_POWER                  0x14
#define MT_SYS_JAMMER_PARAMETERS             0x15
#define MT_SYS_SNIFFER_PARAMETERS            0x16

/* AREQ to host */
#define MT_SYS_RESET_IND                     0x80
#define MT_SYS_OSAL_TIMER_EXPIRED            0x81
#define MT_SYS_JAMMER_IND                    0x82
   
   
#define MT_SYS_RESET_HARD     0
#define MT_SYS_RESET_SOFT     1
#define MT_SYS_RESET_SHUTDOWN 2
   
#define MT_SYS_SNIFFER_DISABLE       0
#define MT_SYS_SNIFFER_ENABLE        1
#define MT_SYS_SNIFFER_GET_SETTING   2
  



// 本地宏
#define MTSYS_SendAsynchData(cmdID,dat,len)     NPISendAsynchData(MT_RPC_SYS_SYS, cmdID, dat, len)
#define MTSYS_SendSynchData(cmdID,dat,len)      NPISendSynchData(MT_RPC_SYS_SYS, cmdID, dat, len)

// async command 
int mtsys_Reset(uint8_t type);

// sync command 
#define mtsys_Ping() MTSYS_SendSynchData(MT_SYS_PING, NULL, 0)



int mtsys_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtsys_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
    

#endif

