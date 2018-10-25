
/*
 * The general format of an FLOW message is:
 * <preamble1><preamble2>,<data length><command>,<data>,<CRC/FCS/SUM>
 * 
 * pdu = <command>,<data>
 *
 * Where:
 * preamble     = the ASCII characters '$M'
 * data length  = number of data bytes, binary. Can be zero as in the case of a data request to the master
 * data         = as per the table below. UINT8
 * CRC          = CRC:
*/

#ifndef __FLOW_DEF_H_
#define __FLOW_DEF_H_

#include "verify.h" 

//frame head offset
#define FLOW_FRAME_HEAD_PREAMBLE1_OFFSET  0  
#define FLOW_FRAME_HEAD_PREAMBLE2_OFFSET  1
// frame data length offset
#define FLOW_FRAME_DATALEN_OFFSET     2

#define FLOW_FRAME_HEAD_LEN               2  // frame head len
#define FLOW_FRAME_DATALEN_LEN            1  // frame data length len
#define FLOW_FRAME_CRC_LEN                2  // CRC
                                  //ascii
#define FLOW_PREAMBLE1       '$'  // 0x24
#define FLOW_PREAMBLE2       'M'  // 0x4d

#endif

