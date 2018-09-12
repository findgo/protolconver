

#ifndef __LTL_GENATTR_H__
#define __LTL_GENATTR_H__

#include "common_type.h"
#include "common_def.h"
#include "ltl.h"
#include "ltl_general.h"

#define ATTRID_BASIC_LTL_VERSION        0x0000
#define ATTRID_BASIC_APPL_VERSION       0x0001
#define ATTRID_BASIC_HW_VERSION         0x0002
#define ATTRID_BASIC_MANUFACTURER_NAME  0x0003
#define ATTRID_BASIC_BUILDDATE_CODE     0x0004
#define ATTRID_BASIC_PRODUCT_ID         0x0005
#define ATTRID_BASIC_SERIAL_NUMBER      0x0006
#define ATTRID_BASIC_POWER_SOURCE       0x0007

#define MANUFACTURER_NAME_STRING_MAX_LEN 16
// for basic power source
#define POWERSOURCE_UNKOWN              0x00
#define POWERSOURCE_SINGLE_PHASE        0x01
#define POWERSOURCE_THREE_PHASE         0x02
#define POWERSOURCE_DC                  0x03
#define POWERSOURCE_BATTERY             0x04
#define POWERSOURCE_EMERGENCY           0x05
// Bit b7 indicates whether the device has a secondary power source in the form of a battery backup

/*** Power Source Attribute bits  ***/
#define POWER_SOURCE_PRIMARY                              0x7F
#define POWER_SOURCE_SECONDARY                            0x80


// ltl version
#define LTL_VERSION     0x01

// app version 
// applition major version 3 bits ,max 7
#define APPL_MAJOR_VERSION      ( 0 )  
// applition minor version 5 bits ,max 31
#define APPL_MINOR_VERSION      ( 0 )
// applition fixed version 5 bits, max 31
#define APPL_FIXED_VERSION      ( 0 )
// applition beta version 3 bits, max 7
// if release set 0
#define APPL_BETA_VERSION        ( 1 )

// hardware version 
// applition major version 3 bits ,max 7
#define HW_MAJOR_VERSION      ( 0 )  
// applition minor version 5 bits ,max 31
#define HW_MINOR_VERSION      ( 0 )
// applition fixed version 5 bits, max 31
#define HW_FIXED_VERSION      ( 0 )
// applition beta version 3 bits, max 7
// if release set 0
#define HW_BETA_VERSION        ( 1 )

//Manufacturer Name ,max 16 bytes
#define MANUFACTURER_NAME  "lchtime"

// product identifier
#define PRODUCT_IDENTIFIER    ( 1 )


void ltl_GeneralAttributeInit(void);



#endif
