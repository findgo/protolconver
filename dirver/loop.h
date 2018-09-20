#ifndef __LOOP_H__
#define __LOOP_H__

#include "common_type.h"
#include "common_def.h"


typedef void (*pTaskFn_t)( void );



void loop_init_System(void);
void loop_Run_System(void);

#endif

