#ifndef __SHT__H_
#define __SHT__H_

#include "app_cfg.h"

#define SHT1x_MOD   0
#define SHT2x_MOD   1

// 选择上面宏的一个
#define SHT_MOD  SHT2x_MOD
	
#if SHT_MOD == SHT1x_MOD
#include "SHT/SHT1x.h"
#endif

#if SHT_MOD == SHT2x_MOD
#include "SHT/SHT2x.h"
#endif

/*************************************************************/
uint8_t bsp_InitSHT(void);
void SHT_PeriodicHandle(void);

#endif
