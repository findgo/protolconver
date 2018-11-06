#ifndef __MT_ZDO_H__
#define __MT_ZDO_H__

#include "mt_npi.h"


/***************************************************************************************************
 * ZDO COMMANDS
 ***************************************************************************************************/

/* SREQ/SRSP */
#define MT_ZDO_NWK_ADDR_REQ                  0x00
#define MT_ZDO_IEEE_ADDR_REQ                 0x01
#define MT_ZDO_NODE_DESC_REQ                 0x02
#define MT_ZDO_POWER_DESC_REQ                0x03
#define MT_ZDO_SIMPLE_DESC_REQ               0x04
#define MT_ZDO_ACTIVE_EP_REQ                 0x05
#define MT_ZDO_MATCH_DESC_REQ                0x06
#define MT_ZDO_COMPLEX_DESC_REQ              0x07
#define MT_ZDO_USER_DESC_REQ                 0x08
#define MT_ZDO_END_DEV_ANNCE                 0x0A
#define MT_ZDO_USER_DESC_SET                 0x0B
#define MT_ZDO_SERVICE_DISC_REQ              0x0C
#define MT_ZDO_END_DEVICE_TIMEOUT_REQ        0x0D
#define MT_ZDO_END_DEV_BIND_REQ              0x20
#define MT_ZDO_BIND_REQ                      0x21
#define MT_ZDO_UNBIND_REQ                    0x22

#define MT_ZDO_SET_LINK_KEY                  0x23
#define MT_ZDO_REMOVE_LINK_KEY               0x24
#define MT_ZDO_GET_LINK_KEY                  0x25
#define MT_ZDO_NWK_DISCOVERY_REQ             0x26
#define MT_ZDO_JOIN_REQ                      0x27
#define MT_ZDO_SEND_DATA                     0x28
#define MT_ZDO_NWK_ADDR_OF_INTEREST_REQ      0x29

#define MT_ZDO_MGMT_NWKDISC_REQ              0x30
#define MT_ZDO_MGMT_LQI_REQ                  0x31
#define MT_ZDO_MGMT_RTG_REQ                  0x32
#define MT_ZDO_MGMT_BIND_REQ                 0x33
#define MT_ZDO_MGMT_LEAVE_REQ                0x34
#define MT_ZDO_MGMT_DIRECT_JOIN_REQ          0x35
#define MT_ZDO_MGMT_PERMIT_JOIN_REQ          0x36
#define MT_ZDO_MGMT_NWK_UPDATE_REQ           0x37

/* AREQ optional, but no AREQ response. */
#define MT_ZDO_MSG_CB_REGISTER               0x3E
#define MT_ZDO_MSG_CB_REMOVE                 0x3F
#define MT_ZDO_STARTUP_FROM_APP              0x40

/* AREQ from host */
#define MT_ZDO_AUTO_FIND_DESTINATION_REQ     0x41
#define MT_ZDO_SEC_ADD_LINK_KEY              0x42
#define MT_ZDO_SEC_ENTRY_LOOKUP_EXT          0x43
#define MT_ZDO_SEC_DEVICE_REMOVE             0x44
#define MT_ZDO_EXT_ROUTE_DISC                0x45
#define MT_ZDO_EXT_ROUTE_CHECK               0x46
#define MT_ZDO_EXT_REMOVE_GROUP              0x47
#define MT_ZDO_EXT_REMOVE_ALL_GROUP          0x48
#define MT_ZDO_EXT_FIND_ALL_GROUPS_ENDPOINT  0x49
#define MT_ZDO_EXT_FIND_GROUP                0x4A
#define MT_ZDO_EXT_ADD_GROUP                 0x4B
#define MT_ZDO_EXT_COUNT_ALL_GROUPS          0x4C
#define MT_ZDO_EXT_RX_IDLE                   0x4D
#define MT_ZDO_EXT_UPDATE_NWK_KEY            0x4E
#define MT_ZDO_EXT_SWITCH_NWK_KEY            0x4F
#define MT_ZDO_EXT_NWK_INFO                  0x50
#define MT_ZDO_EXT_SEC_APS_REMOVE_REQ        0x51
#define MT_ZDO_FORCE_CONCENTRATOR_CHANGE     0x52
#define MT_ZDO_EXT_SET_PARAMS                0x53

/* AREQ to host */
#define MT_ZDO_AREQ_TO_HOST                  0x80 /* Mark the start of the ZDO CId AREQs to host. */
#define MT_ZDO_NWK_ADDR_RSP                  0x80 // ((uint8)NWK_addr_req | 0x80)
#define MT_ZDO_IEEE_ADDR_RSP                 0x81 // ((uint8)IEEE_addr_req | 0x80)
#define MT_ZDO_NODE_DESC_RSP                 0x82 // ((uint8)Node_Desc_req | 0x80)
#define MT_ZDO_POWER_DESC_RSP                0x83 // ((uint8)Power_Desc_req | 0x80)
#define MT_ZDO_SIMPLE_DESC_RSP               0x84 // ((uint8)Simple_Desc_req | 0x80)
#define MT_ZDO_ACTIVE_EP_RSP                 0x85 // ((uint8)Active_EP_req | 0x80)
#define MT_ZDO_MATCH_DESC_RSP                0x86 // ((uint8)Match_Desc_req | 0x80)

#define MT_ZDO_COMPLEX_DESC_RSP              0x87
#define MT_ZDO_USER_DESC_RSP                 0x88
//                                        /* 0x92 */ ((uint8)Discovery_Cache_req | 0x80)
#define MT_ZDO_USER_DESC_CONF                0x89
#define MT_ZDO_SERVER_DISC_RSP               0x8A

#define MT_ZDO_END_DEVICE_TIMEOUT_RSP     0x9F //  ((uint8)End_Device_Timeout_req | 0x80)

#define MT_ZDO_END_DEVICE_BIND_RSP        0xA0 // ((uint8)End_Device_Bind_req | 0x80)
#define MT_ZDO_BIND_RSP                   0xA1 // ((uint8)Bind_req | 0x80)
#define MT_ZDO_UNBIND_RSP                 0xA2 // ((uint8)Unbind_req | 0x80)

#define MT_ZDO_MGMT_NWK_DISC_RSP          0xB0 // ((uint8)Mgmt_NWK_Disc_req | 0x80)
#define MT_ZDO_MGMT_LQI_RSP               0xB1 // ((uint8)Mgmt_Lqi_req | 0x80)
#define MT_ZDO_MGMT_RTG_RSP               0xB2 // ((uint8)Mgmt_Rtg_req | 0x80)
#define MT_ZDO_MGMT_BIND_RSP              0xB3 // ((uint8)Mgmt_Bind_req | 0x80)
#define MT_ZDO_MGMT_LEAVE_RSP             0xB4 // ((uint8)Mgmt_Leave_req | 0x80)
#define MT_ZDO_MGMT_DIRECT_JOIN_RSP       0xB5 // ((uint8)Mgmt_Direct_Join_req | 0x80)
#define MT_ZDO_MGMT_PERMIT_JOIN_RSP       0xB6 // ((uint8)Mgmt_Permit_Join_req | 0x80)

//                                        /* 0xB8 */ ((uint8)Mgmt_NWK_Update_req | 0x80)

#define MT_ZDO_STATE_CHANGE_IND              0xC0
#define MT_ZDO_END_DEVICE_ANNCE_IND          0xC1
#define MT_ZDO_MATCH_DESC_RSP_SENT           0xC2
#define MT_ZDO_STATUS_ERROR_RSP              0xC3
#define MT_ZDO_SRC_RTG_IND                   0xC4
#define MT_ZDO_BEACON_NOTIFY_IND             0xC5
#define MT_ZDO_JOIN_CNF                      0xC6
#define MT_ZDO_NWK_DISCOVERY_CNF             0xC7
#define MT_ZDO_CONCENTRATOR_IND_CB           0xC8
#define MT_ZDO_LEAVE_IND                     0xC9
#define MT_ZDO_TC_DEVICE_IND                 0xCA
#define MT_ZDO_PERMIT_JOIN_IND               0xCB

#define MT_ZDO_MSG_CB_INCOMING               0xFF

// Some arbitrarily chosen value for a default error status msg.
#define MtZdoDef_rsp                         0x0040


// 本地宏
#define MTZDO_SendAsynchData(cmdID,dat,len)     NPISendAsynchData(MT_RPC_SYS_ZDO, cmdID, dat, len)
#define MTZDO_SendSynchData(cmdID,dat,len)      NPISendSynchData(MT_RPC_SYS_ZDO, cmdID, dat, len)

// sync command 



int mtzdo_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtzdo_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);

#endif

