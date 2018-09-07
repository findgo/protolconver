

#include <stdio.h>
#include <stdarg.h>
#include "log.h"

// 默认输出到stdout
static void mo_log_defaultlog_callback(void *ctx, log_level_t level, const char *format,...)
{
    (void)ctx;
    (void)level;
    va_list ap;

    va_start(ap,format);
	printf(format,va_arg(ap,char *));
	va_end(ap);
}

//将所以信息默
static void mo_log_nulllog_callback(void *ctx, log_level_t level, const char *format,...)
{
    (void)ctx;
    (void)level;
    (void)format;
}

logger_t default_logger = {
	MO_LEVEL_ERROR,
    NULL,
	mo_log_defaultlog_callback
};

/*********************************************************************
 * @fn          
 *
 * @brief       注册一个
 *
 * @param       log_Funcpfn_t - function pointer to read/write routine
 *
 * @return      
 */
void mo_log_set_logger_callback(log_FuncpfnCB_t log_func)
{
    if (NULL == log_func)
        log_func = mo_log_nulllog_callback;
    else if(log_func == MO_LOG_DEFAULTLOG_CB)
        log_func = mo_log_defaultlog_callback;

    default_logger.log = log_func;
}

