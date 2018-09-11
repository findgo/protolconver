#ifndef __LTL_GENERAL_H__
#define __LTL_GENERAL_H__

#include "common_type.h"
#include "common_def.h"
#include "ltl_trunk.h"
#include "ltl.h"

// This callback is called to process an Reset to Factory Defaults
// command. On receipt of this command, the device resets all the attributes
// of all its clusters to their factory defaults.
typedef void(*ltlGenCB_BasicResetFact_t) (uint8_t nodeNo);
typedef void(*ltlGenCB_BasicReboot_t)(void);
typedef void (*ltlGenCB_Identify_t)( uint16_t identifyTime );


typedef struct
{
    ltlGenCB_BasicResetFact_t           pfnBasicResetFact;                // Basic trunk Reset command
    ltlGenCB_BasicReboot_t              pfnBasicReboot;                // Basic trunk reboot command
    ltlGenCB_Identify_t                 pfnIdentify;                  // Identify command
} ltlGeneral_AppCallbacks_t;





#endif 

