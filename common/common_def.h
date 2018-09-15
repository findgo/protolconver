
/**
  ******************************************************************************
  * @file   commond_def.h
  * @author  
  * @version 
  * @date    
  * @brief    é€šç”¨æ€§å®šä¹‰,ç”¨äºæ•°æ®å¤„ç†
  ******************************************************************************
  * @attention  20151110     v1.1       jgb     ÖØ¹¹
  * @attention  20180906     v1.2       jgb     è„±ç¦»å¹³å°é™åˆ¶
  ******************************************************************************
  */



#ifndef __COMMON_DEF_H_
#define __COMMON_DEF_H_

#include "common_type.h"
#include "common_signal.h"
/* ------------------------------------------------------------------------------------------------
 *                                             Macros
 * ------------------------------------------------------------------------------------------------
 */

#ifndef BV
#define BV(n)      (1 << (n))
#endif

#ifndef BF
#define BF(x,b,s)  (((x) & (b)) >> (s))
#endif

#ifndef MIN
#define MIN(n,m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n,m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
#define ABS(n)     (((n) < 0) ? -(n) : (n))
#endif

#ifndef UBOUND
#define UBOUND(__ARRAY) (sizeof(__ARRAY)/sizeof(__ARRAY[0]))
#endif

//!< 
#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((uint32_t)((uint32_t)((Byte0) & 0x00FF) \
          + ((uint32_t)((Byte1) & 0x00FF) << 8) \
          + ((uint32_t)((Byte2) & 0x00FF) << 16) \
          + ((uint32_t)((Byte3) & 0x00FF) << 24)))

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define BUILD_UINT8(loByte, hiByte) ((uint8_t)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define HI_UINT8(a) (((a) >> 4) & 0x0F)
#define LO_UINT8(a) ((a) & 0x0F)

/* takes a byte out of a uint32_t : var - uint32_t,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) (uint8_t)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))
/* takes a byte out of a uint64_t : var - uint64_t,  ByteNum - byte to take out (0 - 7) */
#define BREAK_UINT64( var, ByteNum ) (uint8_t)((uint64_t)(((var) >>((ByteNum) * 8)) & 0x00FF))

//!< 
#define SWAP8_U16(v)                    ((((uint16_t)(v) & 0xFF00) >> 8) | \
                                        (((uint16_t)(v) & 0x00FF) << 8))
#define SWAP8_U24(v)                    ((((uint32_t)(v) & 0x00FF0000) >> 16) | \
                                        (((uint32_t)(v) & 0x0000FF00) << 0) | \
                                        (((uint32_t)(v) & 0x000000FF) << 16))
#define SWAP8_U32(v)                    ((((uint32_t)(v) & 0xFF000000) >> 24) | \
                                        (((uint32_t)(v) & 0x00FF0000) >> 8) | \
                                        (((uint32_t)(v) & 0x0000FF00) << 8) | \
                                        (((uint32_t)(v) & 0x000000FF) << 24))


//! \name binary compatibility  
#define USE_SET_BIT(__x, BitNum)    ((__x) |= (BV(BitNum)))
#define USE_CLR_BIT(__x, BitNum)    ((__x) &= ~(BV(BitNum)))
#define USE_CPL_BIT(__x ,BitNum)    ((__x) ^= (BV(BitNum)))
#define USE_GET_BIT(__x, BitNum)    (((__x) >> (BitNum)) & 0x01)

//½«Ò»¸ö×ÖÄ¸×ª»»Îª´óĞ´
#define  UPCASE( c )    (((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )
    
//ÅĞ¶Ï×Ö·ûÊÇ²»ÊÇ10½øÖµµÄÊı×Ö
#define  DECCHK( c )    ((c) >= '0' && (c) <= '9')
//ÅĞ¶Ï×Ö·ûÊÇ²»ÊÇ16½øÖµµÄÊı×Ö
#define  HEXCHK( c )    ( ((c) >= '0' && (c) <= '9') ||\
                        ((c) >= 'A' && (c) <= 'F') ||\
                        ((c) >= 'a' && (c) <= 'f') )

#define  MEM_B( x )  ( *( (uint8_t *) (x) ) )//µÃµ½Ö¸¶¨µØÖ·ÉÏµÄÒ»¸öbyte
#define  MEM_W( x )  ( *( (uint16_t *) (x) ) )//µÃµ½Ö¸¶¨µØÖ·ÉÏµÄÁ½¸öbyte

//µÃµ½Ò»¸öfieldÔÚ½á¹¹Ìå(struct)ÖĞµÄÆ«ÒÆÁ¿
#define FPOS( type, field ) ( (uint32_t) &(( type *) 0)-> field )
//µÃµ½Ò»¸ö½á¹¹ÌåÖĞfieldËùÕ¼ÓÃµÄ×Ö½ÚÊı
#define FSIZ( type, field ) sizeof( ((type *) 0)->field )



//æä¾›ä¸­æ–­çº§ä¸´ç•Œä¿æŠ¤å®
// set a save variable and enter exit criticial macro
// portCriticial_state_Variable: å®šä¹‰ä¸€ä¸ªä¿å­˜å˜é‡
#define portCriticial_state_Variable    //halIntState_t bintstate
#define portCriticial_Enter_code()      //ENTER_SAFE_ATOM_CODE(bintstate)
#define portCriticial_Exit_code()       //EXIT_SAFE_ATOM_CODE(bintstate)



#define configASSERT( x )







#endif




