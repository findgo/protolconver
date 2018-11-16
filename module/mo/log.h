
/*
 *  realize four level log module
 *
 * 从小到大排序 error, warn, info, debug
 * 默认输出级别最大warn, 比warn低可以输出
 *
 * 
 */


#ifndef __MO_LOG_H__
#define __MO_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "common_type.h"
#include "mserial.h"
#include "usart.h"
enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};


#define log_error(format,args...)       log_ll(LOG_LEVEL_ERROR,format,##args)
#define log_warn(format,args...)        log_ll(LOG_LEVEL_WARN,format,##args)
#define log_info(format,args...)        log_ll(LOG_LEVEL_INFO,format,##args)
#define log_debug(format,args...)       log_ll(LOG_LEVEL_DEBUG,format,##args)

#define log_errorln(format,args...)     log_llln(LOG_LEVEL_ERROR,format,##args)
#define log_warnln(format,args...)      log_llln(LOG_LEVEL_WARN,format,##args)
#define log_infoln(format,args...)      log_llln(LOG_LEVEL_INFO,format,##args)
#define log_debugln(format,args...)     log_llln(LOG_LEVEL_DEBUG,format,##args)


void log_set_max_level(uint8_t level);
void log_Init(void);
void log_llln(uint8_t level,const char *format,...);
void log_ll(uint8_t level,const char *format,...);


// 定义一个外部初始化
#define lowlogInit() do {SerialDrvInit(COM2, 115200, 8, DRV_PAR_NONE); log_set_max_level(LOG_LEVEL_DEBUG); }while(0)
// 重定向fput的字符
#define lowlogputChar(c) Serial_WriteByte(COM2,c);


#ifdef __cplusplus
}  /* extern "C */
#endif

#endif

