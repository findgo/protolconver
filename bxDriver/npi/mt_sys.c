#include "mt_sys.h"


int mtsys_Reset(uint8_t type)
{
    return MTSYS_SendAsynchData(MT_SYS_RESET_REQ, &type, 1);
}

int mtsys_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{

}


int mtsys_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    
}
