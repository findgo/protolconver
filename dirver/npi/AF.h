
#ifndef __AF_H__
#define __AF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"


/*********************************************************************
 * Simple Descriptor
 */

// Generalized MSG Command Format
typedef struct
{
  uint8_t   TransSeqNumber;
  uint16_t  DataLength;              // Number of bytes in TransData
  uint8_t  *Data;
} afMSGCommandFormat_t;

typedef enum
{
  noLatencyReqs,
  fastBeacons,
  slowBeacons
} afNetworkLatencyReq_t;

/*********************************************************************
 * Endpoint  Descriptions
 */

typedef enum
{
  afAddrNotPresent = AddrNotPresent,
  afAddr16Bit      = Addr16Bit,
  afAddr64Bit      = Addr64Bit,
  afAddrGroup      = AddrGroup,
  afAddrBroadcast  = AddrBroadcast
} afAddrMode_t;

typedef struct
{
  union
  {
    uint16_t      shortAddr;
    ZLongAddr_t extAddr;
  } addr;
  afAddrMode_t addrMode;
  uint8_t endPoint;
  uint16_t panId;  // used for the INTER_PAN feature
} afAddrType_t;


typedef struct
{
  uint16_t groupId;           /* Message's group ID - 0 if not set */
  uint16_t clusterId;         /* Message's cluster ID */
  afAddrType_t srcAddr;     /* Source Address, if endpoint is STUBAPS_INTER_PAN_EP,
                               it's an InterPAN message */
  uint16_t macDestAddr;       /* MAC header destination short address */
  uint8_t endPoint;           /* destination endpoint */
  uint8_t wasBroadcast;       /* TRUE if network destination was a broadcast address */
  uint8_t LinkQuality;        /* The link quality of the received data frame */
  uint8_t correlation;        /* The raw correlation value of the received data frame */
  int8  rssi;               /* The received RF power in units dBm */
  uint8_t SecurityUse;        /* deprecated */
  uint32 timestamp;         /* receipt timestamp from MAC */
  uint8_t nwkSeqNum;          /* network header frame sequence number */
  afMSGCommandFormat_t cmd; /* Application Data */
  uint16_t macSrcAddr;        /* MAC header source short address */
} afIncomingMSGPacket_t;

// Reflect Error Message - sent when there is an error occurs
// during a reflected message.
typedef struct
{
  uint8_t endpoint;        // destination endpoint
  uint8_t transID;         // transaction ID of sent message
  uint8_t dstAddrMode;     // destination address type: 0 - short address, 1 - group address
  uint16_t dstAddr;        // destination address - depends on dstAddrMode
} afReflectError_t;

// Endpoint Table - this table is the device description
// or application registration.
// There will be one entry in this table for every
// endpoint defined.
typedef uint16_t  cId_t;
typedef struct
{
  uint8_t           endPoint;
  uint16_t         AppProfId;
  uint16_t         AppDeviceId;
  uint8_t          AppDevVer;
  afNetworkLatencyReq_t latencyReq;
  uint8_t          AppNumInClusters;
  uint8_t          AppNumOutClusters;
  cId_t         *pAppInClusterList;
  cId_t         *pAppOutClusterList;
} endPointDesc_t;

#ifdef __cplusplus
}
#endif
#endif
/**************************************************************************************************
*/

