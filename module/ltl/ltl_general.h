#ifndef __LTL_GENERAL_H__
#define __LTL_GENERAL_H__

#include "common_type.h"
#include "common_def.h"
#include "ltl_trunk.h"
#include "ltl.h"

// This callback is called to process an Reset to Factory Defaultscommand. 
// On receipt of this command, the device resets all the attributes of all its clusters to their factory defaults.
typedef void(*ltlGenCB_BasicResetFact_t) (uint8_t nodeNo);
// This callback is called to process an reboot the device. 
typedef void(*ltlGenCB_BasicReboot_t)(void);
// This callback is called to process an incoming Identify command.
//   identifyTime - number of miniseconds the device will continue to identify itself
typedef void (*ltlGenCB_Identify_t)( uint16_t identifyTime );
// This callback is called to process an incoming On, Off or Toggle command.
//   cmd - received command, which will be either COMMAND_ONOFF_ON, COMMAND_ONOFF_OFF
//         or COMMAND_ONOFF_TOGGLE.
typedef void (*ltlGenCB_Onoff_t)(uint8_t cmd);

typedef struct
{
    ltlGenCB_BasicResetFact_t           pfnBasicResetFact;                // Basic trunk Reset command
    ltlGenCB_BasicReboot_t              pfnBasicReboot;                // Basic trunk reboot command
    ltlGenCB_Identify_t                 pfnIdentify;                  // Identify command
    ltlGenCB_Onoff_t                    pfnOnoff;
} ltlGeneral_AppCallbacks_t;





#endif 

