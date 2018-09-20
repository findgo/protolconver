/* 提供中断级临界保护 */

#include "port_atom.h"
#include "common_type.h"
#include "common_def.h"

/* Each task maintains its own interrupt status in the critical nesting variable. */
static uint32_t CriticalNesting = 0;

void PortAtomEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	CriticalNesting++;

	if( CriticalNesting == 1 ) {
		configASSERT( ( ( * ( ( volatile uint32_t * ) 0xe000ed04 ) ) & 0xff ) == 0 );
	}
}

/*-----------------------------------------------------------*/

void PortAtomExitCritical( void )
{
    configASSERT( CriticalNesting );
    
    CriticalNesting--;
    if( CriticalNesting == 0 ) {
        portENABLE_INTERRUPTS();
    }
}

