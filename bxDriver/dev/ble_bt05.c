

#include <stdio.h>
#include "ble_bt05"

#define BLE_BT05_BUFF_LEN       32


typedef void At_cmd_handler_t(uint8_t argc, char *argv[], void *pcmd);

typedef struct {
	const char *pchCMD;
	At_cmd_handler_t *cmd_handler;
}At_commandparse_t;


typedef struct 
{
    uint8_t currentState;  // 处于低功耗还是工作状态,仅低功耗模式有效

    char ver[16];  // 软件版本
    char addrTab[6];
    char nameTab[20];
    int pin;

    uint8_t serialBaud;
    uint8_t serialStopBit;
    uint8_t serialPari;
    
    uint8_t role;       // 角色:  主机/从机
    uint8_t PWRMmode;  // 低功耗,正常工作


    char *devlist[];
}ble_data_t;

ble_data_t bledata;


static const At_commandparse_t cmdparsingTal[] = {
    {AT_AT, NULL},
    {AT_RESET, NULL},
    {AT_DEFAULT, NULL},
    {AT_VERSION, NULL},
    {AT_LADDR, NULL},
    {AT_NAME, NULL},
    {AT_PIN, NULL},
    {AT_BAUD, NULL},
    {AT_STOP, NULL},
    {AT_PARI, NULL},
    {AT_PWRM, NULL},
    {AT_SLEEP, NULL},
    {AT_POWE, NULL},
    {AT_ROLE, NULL},
    {AT_INQ, NULL},
    {AT_SHOW, NULL},
    {AT_CONN, NULL},
    {AT_HELP, NULL},
};
    
static const uint32_t At_bandrateTab[] = {9600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400};


static char ble_buf[BLE_BT05_BUFF_LEN];

void ble_bt05Request(char *str, char *para)
{
    memset(ble_buf,'\0',sizeof(ble_buf));
    sprintf(ble_buf,"%s%s%s%s",AT_AT, (str ==NULL)  ? "" : str, (para ==NULL)  ? "" : para, AT_CRLF);
 /*   
    strcat(ble_buf,AT_AT);
	strcat(ble_buf,(str ==NULL)  ? "" : str);
    strcat((para ==NULL)  ? "" : para);
	strcat(ble_buf,AT_CRLF);
*/
    Ble_WriteStr(ble_buf);
}

static void ble_nullHandle(uint8_t argc, char *argv[], void *pcmd)
{
    // nothing to do
    (void)argc;
    (void)argv;
}

static void ble_versionHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;

    strcpy(bledata.pver, argv[0]);
}

static void ble_laddrhandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;
    //格式    11:22:33:44:55:66
    //strcpy(bledata.addrTab, argv[0]);
}

static void ble_nameHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;

    strcpy(bledata.nameTab, argv[0]);
}
static void ble_pinHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;
    //格式 000000

     bledata.pin = atoi(argv[0]);
}
static void ble_baudHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;
    // the index of bandTable    
     bledata.serialBaud = atoi(argv[0]);    
}
static void ble_stopbitHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;

     bledata.serialStopBit = atoi(argv[0]);    
}
static void ble_pariHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;

     bledata.serialStopBit = atoi(argv[0]);    
}

static void ble_PWRMHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;

     bledata.PWRMmode = atoi(argv[0]);    
}

static void ble_roleHandle(uint8_t argc, char *argv[], void *pcmd)
{
    (void)argc;

     bledata.role = atoi(argv[0]);    
}
