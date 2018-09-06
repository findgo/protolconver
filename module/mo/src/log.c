

#include <stdio.h>
#include <stdarg.h>
#include "log.h"

static void mo_log_default_log(void *ctx, log_level_t level, const char *format,...)
{
    (void)ctx;
    (void)level;
    va_list ap;

    va_start(ap,format);
	printf(format,va_arg(ap,char *));
	va_end(ap);
}

static void mo_log_null_log(void *ctx, log_level_t level, const char *format,...)
{
    (void)ctx;
    (void)level;
    (void)format;
}

logger_t default_logger = {
	MO_LEVEL_ERROR,
    NULL,
	mo_log_default_log
};

void mo_log_set_logger(void *context, log_Funcpfn_t log_func)
{
    if (NULL == log_func)
        log_func = mo_log_null_log;

    default_logger.context = context;
    default_logger.log = log_func;
}

