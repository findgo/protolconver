

#ifndef __LTL_TRUNK_H__
#define __LTL_TRUNK_H__

#define LTL_DEVICE_COMMON_NODENO    0x00

#define LTL_TRUNK_ID_GENERAL_BASIC  0x0000
#define LTL_TRUNK_ID_GENERAL_POWER  0x0001
#define LTL_TRUNK_ID_GENERAL_ONOFF  0x0002


// for LTL_TRUNK_ID_GENERAL_BASIC specific command
#define COMMAND_BASIC_RESET_FACT_DEFAULT 0x00
#define COMMAND_BASIC_REBOOT_DEVICE     0x01
#define COMMAND_BASIC_IDENTIFY          0x02

// for LTL_TRUNK_ID_GENERAL_ONOFF specific command
#define COMMAND_ONOFF_OFF               0x00
#define COMMAND_ONOFF_ON                0x01
#define COMMAND_ONOFF_TOGGLE            0x02



#endif
