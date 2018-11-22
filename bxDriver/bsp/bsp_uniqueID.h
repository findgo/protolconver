


#ifndef __BSP_UNIGUEID_H_
#define __BSP_UNIGUEID_H_

#include "common_type.h"
#include "common_def.h"

typedef struct {
    uint32_t inflashsize; // xxx K
    uint32_t id0;
    uint32_t id1;
    uint32_t id2;
}mcu_info_t;

mcu_info_t *bsp_GetChipInfo(void);

#endif

