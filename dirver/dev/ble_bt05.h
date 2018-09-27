
/*
 *  指示蓝牙所处状态
 *  模式              LED 显示                                模块状态
 *  主模式           均匀闪烁(300ms-on,300ms-off)                搜索及连接中
 *                长亮                                      建立连接
 *  从模式           均匀慢速闪烁(800ms-on,800ms-off)              等待配对
 *                长亮                                      建立连接
 *
 * 唤醒模块有三种方式:
 * 方法一：您可以通过发送长度 80 左右的字符串来激活模块。 发送的这个激活字符
 *  串不能包含 AT 指令，成功唤醒后，串口将会输出“+WAKE\r\nOK\r\n”字符串。
 *  方法二：短按系统按键SW1。
 *  方法三：直接配对连接。
*/


#ifndef __BLE_DXBT05_H__
#define __BLE_DXBT05_H__

#include "common_type.h"

#define Atstr(str)  #str

#define AT_CRLF         Atstr(\r\n)     // 指令回车,换行结尾字符
// AT指令只能在模块未连接状态下才能生效,一旦连接上,模块进行数据透传模式
#define AT_AT           Atstr(AT)       // 测试指令 rsp ---> OK
#define AT_RESET        Atstr(+RESET)   // 复位指令 500ms重启 rsp ---> OK
#define AT_DEFAULT      Atstr(+DEFAULT) // 重置指令  500ms后恢复默认设置           rsp ---> OK
#define AT_VERSION      Atstr(+VERSION) // 获取版本号         rsp---> =<Param>
#define AT_LADDR        Atstr(+LADDR)   // 模块蓝牙地址        rsp---> =<Param>
#define AT_NAME         Atstr(+NAME)    // 设置/查询设备名称, 重启生效             
                                        // 查询 rsp---> =<Param>    
                                        // 设置 rsp---> =<Param>,OK
#define AT_PIN          Atstr(+PIN)     // 设置/查询配对码, 重启生效             
                                        // 查询 rsp---> =<Param>   /* 6位配对码 默认000000*/  
                                        // 设置 rsp---> =<Param>,OK
#define AT_BAUD         Atstr(+BAUD)    // 设置/查询波特率, 立即生效             
                                        // 查询 rsp---> =<Param>   /* 查看波特率映射表 */  
                                        // 设置 rsp---> =<Param>,OK
#define AT_STOP         Atstr(+STOP)    // 设置/查询停止位, 立即生效             
                                        // 查询 rsp---> =<Param>   /* 0: 1停止位, 1:    2停止位 */  
                                        // 设置 rsp---> =<Param>,OK 
#define AT_PARI         Atstr(+PARI)    // 设置/查询校验位, 立即生效             
                                        // 查询 rsp---> =<Param>   /* 0: 无效验, 1: 奇校验       2 : 无效验 */
                                        // 设置 rsp---> =<Param>,OK
#define AT_PWRM         Atstr(+PWRM)    // 设置上电是否进入低功耗, 可配对 立即生效             
                                        // 查询 rsp---> =<Param>   /* 0: 进入低功耗, 1: 正常工作 默认: 1*/
                                        // 设置 rsp---> =<Param>,OK 
#define AT_SLEEP        Atstr(+SLEEP)   // 进入低功耗 rsp ---> ,OK

#define AT_POWE         Atstr(+POWE)     // 设置/查询模块功率           
                                         // 查询 rsp---> =<Param>   /* 0: -23dB, 1: -6dB, 2: 0dB, 3: +4dB 默认: 2*/
                                         // 设置 rsp---> =<Param>,OK

#define AT_ROLE         Atstr(+ROLE)     // 设置/查询 -- 主/从模式           角色设置完毕会自动重启并生效, 可硬件设置主从,那时只能用查询             
                                        // 查询 rsp---> =<Param>   /* 0: 从设备, 1: 主设备 默认: 0*/
                                        // 设置 rsp---> =<Param>,OK
// for master
#define AT_INQ          Atstr(+INQ)     // 搜索蓝牙设备             
                                        // rsp--->OK, +INQS,+INQ:0 dev0,INQ:1 dev1,....,+INQE
#define AT_SHOW         Atstr(+SHOW)    // 显示搜索到的蓝牙设备
                                        // rsp--->+INQ:0 dev0,INQ:1 dev1,....
#define AT_CONN         Atstr(+CONN)    // 连接远端设备 
                                        // rsp: +CONNECTING>>dev0  连接中
                                        // rsp: +CONNECTED>>dev0  已连接
#define AT_HELP         Atstr(+HELP)    // 帮助

#define AT_RSP_OK       Atstr(OK)
#define AT_RSP_ERR      Atstr(ERR)

#define BLE_SERIAL_BAUD_MIN    1
#define BLE_SERIAL_BAUD_MAX    9

#define BLE_SERIAL_STOPBIT_1    0
#define BLE_SERIAL_STOPBIT_2    1

#define BLE_SERIAL_PARI_NONE    0
#define BLE_SERIAL_PARI_ODD     1
#define BLE_SERIAL_PARI_EVEN    2

#define BLE_MODE_POWER_ON_LOW_POWER      0
#define BLE_MODE_POWER_ON_NROMAL         1

#define BLE_ROLE_SLAVE     0
#define BLE_ROLE_MASTER    1

#define Ble_WriteStr(str)


#define ble_test()                      ble_bt05Request(NULL, NULL)
#define ble_reset()                     ble_bt05Request(AT_RESET, NULL)
#define ble_default()                   ble_bt05Request(AT_DEFAULT, NULL)

#define ble_getVersion()                ble_bt05Request(AT_VERSION, NULL)
#define ble_getLaddress()               ble_bt05Request(AT_LADDR, NULL)
#define ble_getDevName()                ble_bt05Request(AT_NAME, NULL)
#define ble_setDevName(name)            ble_bt05Request(AT_NAME, name)
#define ble_getPin()                    ble_bt05Request(AT_PIN, NULL)
#define ble_setPin(pin)                 ble_bt05Request(AT_PIN, pin)
#define ble_getSerialBaud()             ble_bt05Request(AT_BAUD, NULL)
#define ble_setSerialBaud(baud)         ble_bt05Request(AT_BAUD, baud)
#define ble_getSerialStopBit()          ble_bt05Request(AT_STOP, NULL)
#define ble_setSerialStopBit(stopbit)   ble_bt05Request(AT_STOP, stopbit)
#define ble_getSerialPari()             ble_bt05Request(AT_PARI, NULL)
#define ble_setSerialPari(pari)         ble_bt05Request(AT_PARI, pari)
#define ble_getPWRMmode()               ble_bt05Request(AT_PWRM, NULL)
#define ble_setPWRMmode(mode)           ble_bt05Request(AT_PWRM, mode)
#define ble_sleep()                     ble_bt05Request(AT_SLEEP, NULL)
#define ble_getPowe()                   ble_bt05Request(AT_POWE, NULL)        
#define ble_setPowe(powe)               ble_bt05Request(AT_POWE, powe)    
#define ble_help()                      ble_bt05Request(AT_HELP, NULL)
#define ble_getRole()                   ble_bt05Request(AT_ROLE, NULL)
// for master
#define ble_setRole(role)               ble_bt05Request(AT_ROLE, role)
#define ble_searchDev()                 ble_bt05Request(AT_INQ, NULL)
#define ble_showDev()                   ble_bt05Request(AT_SHOW, NULL)
#define ble_connectDev(devNo)           ble_bt05Request(AT_CONN, devNo)        


void ble_bt05Request(char *str, char *para);


#endif


