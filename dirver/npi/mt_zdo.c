
#include "mt_zdo.h"

extern void zbNwkCheckZdo(uint8_t status);


int mtzdo_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    return NPI_LNX_SUCCESS;
}
int mtzdo_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    switch (commandID){
    case MT_ZDO_STATE_CHANGE_IND:
        zbNwkCheckZdo(data[0]);
        break;
    }
    
    return NPI_LNX_SUCCESS;
}


