
#include "XK2233.h"

uint8_t XK2233Status_state = 0;
//红外接收,发射
XK2233_DATA xk2233data;
//{"db_sbID":"10008","ctrl":{"cmd":0}}
/*学习型
{
	"db_sbID":	10008,
	"payload":	[{
			"ctrl":	{
				"learnmode":1,
				"cmd":	10,
				"position":0,
				"functions":0,
				"times":5
			},
			"clusterID":	1
		}]
}
*/
//uint16_t model; //空调型号
//	char sw;	//FF:开 00:关
//	char mode; //00: 自动 01: 制冷 02: 除湿 03: 送风 04: 制暖
//	char temp; //10H-1EH(16-31℃)
//	char speed; //00 = 自动 01=1 档位 02=2 档位 03=3 档位
//	char wdir; //00 = 自动摆风 01 手动摆风
/*空调型
{
	"db_sbID":	10008,
	"payload":	[{
			"ctrl":	{
				"learnmode":0,
				"cmd":	10,
				"model":121,
				"sw":255,
				"mode":0,
				"temp":16,
				"speed":0,
				"wdir":0
			},
			"clusterID":	1
		}]
}
*/

void XK2233_Periodic_Handle(void)
{
	static uint8_t XK2233_Init_ready_state = 0;
    
	if(XK2233_Init_ready_state != STATUS_OK){ 	

		bsp_InitXK2233();

		XK2233_Init_ready_state =1;
	}	
	
    uint16_t len=0;

    uint8_t chipbuffer[USART5BUFLENTH];/*定义一个2KB的缓存*/

    memset(chipbuffer, 0, sizeof(chipbuffer));
    //接收
    len = UART5_QUEUE_RXPacketLengthGet() ;    
    if(( len > 0))
    {		
    	len= XK2233_RCV(chipbuffer, len); 
    	//如果没有发送命令,就不处理数据
    	if(xk2233data.ready != 1 || len == 0){
    		return;
    	}
    	
    	bool ret;
    	ret = XK2233_readDeviceStatus(chipbuffer, len);
    	if(ret == true){
    		xk2233data.cmdstatus = 1;
    	}
        else{
    		xk2233data.cmdstatus = 0;
    	}
        xk2233data.ready = 0;
    	XK2233Status_state = 1;
    }

    //发送
    len = UART5_QUEUE_TXPacketLengthGet() ;    
    if(( len >=   5)){
    	int i = 0;
    	len= XK2233UART_TxRead(chipbuffer, 5); 
    	printf("\r\nUSART5发送缓存长度%d个数据\r\n",len); 
    	while(len--)
    	{	
    		UART5_putchar(chipbuffer[i++]); 
    	}

    }	
}

bool XK2233_readDeviceStatus(uint8_t* buffer, uint8_t len)
{				
	if(len >= 1){
		//处理字节
		if(buffer[0] == XK2233_EXCUTE_SUCCESS){
			return true;
		}
        else if(buffer[0] == XK2233_EXCUTE_FAILED){
			return false;
		}
		else if(buffer[0] == 0x27){
			return true;
		}	
		else{
			return true;
		}
	}
    
	return false;				
}


static uint8_t Xor(uint8_t *src , uint8_t len)
{
    uint8_t i ;
    uint8_t __sum =0;
    
	for(i = 0; i < len; i++){
		__sum ^= src[i];
	}
    
	return __sum;
}

void Xk2223_request(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) 
{
    uint8_t buffer[5];

    buffer[0] = byte1;
    buffer[1] = byte2;
    buffer[2] = byte3;
    buffer[3] = byte4;
    buffer[4] = Xor(buffer, 4);
    XK2233_SEND(buffer, sizeof(buffer));
}



//学习代号(0x00-0x64)
void XK2233_sendlearningFun(uint8_t learning_code)
{		
    memset((char*)&xk2233data,'\0',sizeof(xk2233data));
    xk2233data.ircmd = XK2233_CMD_LEARNING;
    xk2233data.ready = 1;
    xk2233data.code = learning_code;

    Xk2223_request(XK2233_CMD_LEARNING, learning_code, 0, 0);
}
//发射已学习代号(0x00-0x64)
void XK2233_sendLearnedFun(uint8_t learned_code , uint8_t sw)
{		
    memset((char*)&xk2233data,'\0',sizeof(xk2233data));
    xk2233data.ircmd = XK2233_CMD_LEARNED;
    xk2233data.ready = 1;
    xk2233data.code = learned_code;
    if(sw == 0 || sw == 255){
        xk2233data.sw = sw;
    }

    Xk2223_request(XK2233_CMD_LEARNED, learned_code, 0, 0);
}

//部分空调操作
//设定空调型号
 void XK2233_sendsetACModel(uint16_t model)
{		
		xk2233data.ircmd = XK2233_CMD_SET_MODEL;
		xk2233data.model = model;
		xk2233data.ready = 1;

        Xk2223_request(XK2233_CMD_SET_MODEL,( model >> 8 ) & 0xff, model & 0xff, 0x08);
}

//设定空调开关
 void XK2233_sendACsw(uint8_t sw)
{		
    uint8_t realsw = (sw > 0) ? 0xff : 0x00;
    
    xk2233data.ircmd = XK2233_CMD_SW;
    xk2233data.sw = realsw;
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_SW, realsw, 0x08, 0x08);
}

//设定空调运行模式
 void XK2233_sendACmode(uint8_t mode)
{		
    xk2233data.ircmd = XK2233_CMD_MODE;
    xk2233data.mode = mode;
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_MODE, mode, 0x08, 0x08);
}

//设定空调温度命令
 void XK2233_sendACtemp(uint8_t temp)
{		
    if(temp > XK2233_TEMP_MAX)
        temp = XK2233_TEMP_MAX;
    if(temp < XK2233_TEMP_MIN)
        temp = XK2233_TEMP_MIN;

    xk2233data.ircmd = XK2233_CMD_TEMPERATURE;
    xk2233data.temp = temp;
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_TEMPERATURE, temp, 0x08, 0x08);
}

//设定空调风速命令
 void XK2233_sendACWindspeed(uint8_t speed)
{		
    speed = (speed > XK2233_WINDSPEED_MAX) ? XK2233_WINDSPEED_MAX : speed;
    
    xk2233data.ircmd = XK2233_CMD_WINDSPEED;
    xk2233data.speed = speed;
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_WINDSPEED, speed, 0x08, 0x08);
}

//设定空调风向命令
 void XK2233_sendACWindDir(uint8_t winddir)
{
    winddir = winddir > 0 ? XK2233_WINDDIR_HAND : XK2233_WINDDIR_AUTO;
    
    xk2233data.ircmd = XK2233_CMD_WINDDIR;
    xk2233data.wdir = winddir;
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_WINDDIR, winddir, 0x08, 0x08);
}
//休眠
 void XK2233_sendChipsleep(void)
{
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_CHIPSLEEP, 0xff, 0x08, 0x08);
}
 
//初始化启动命令
 void XK2233_sendChipInitStart(void)
{		
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_CHIPINITSTART, 0xaa, 0x08, 0x08);
}
//结束初始化启动命令
 void XK2233_sendChipInitEnd(void)
{		
    xk2233data.ready = 1;
    Xk2223_request(XK2233_CMD_CHIPINITEND, 0xcc, 0x08, 0x08);
}
/*
void XK2233_PeriodicHandle_1S(void)
{
//		//
//		XK2233_openDevice(10);
//		XK2233_addDevice(120);
//	XK2233_set_ACmodel(11);
//	haierTVVolumeP();
}
void subscribe_topic_IR(void)
{
	char _mqtttopic[100];
	char str_mac[13];
	{//4个初始值
		
		bsp_ChipSerialToMACStr(str_mac);
	}
	sprintf(_mqtttopic,"ctrl/""%d""/""%s""/%d",UI_ID,str_mac,IRDeviceID);
  if(IRDeviceID!=0) mqtt_sub_single_command(_mqtttopic,QOS2);
}
///////////////////json_fun///////////////////

//红外发送和接收主题
//红外开关
void XK2233_ParseJson(char *src)
{
  root = cJSON_Parse(src);
	if(root == NULL) goto exit;

	db_sbID  = cJSON_GetObjectItem(root,DEVICEID);
	if(db_sbID == NULL) goto exit;


	if(db_sbID->model == cJSON_Number)
	{
		if(db_sbID->valueint == IRDeviceID)
//			if(strcmp(pid,S20DeviceID)==0)
		{
			payload = cJSON_GetObjectItem(root,PAYLOAD);
			if(payload == NULL) goto exit;
			
			{
				int i=0;
				int size  = cJSON_GetArraySize(payload);
				for(i=0;i<size;i++)
				{
					subarrayarr[0] = cJSON_GetArrayItem(payload,i);
					if(subarrayarr[0]->model == cJSON_Object)
					{
						subdataarr[0] = cJSON_GetObjectItem(subarrayarr[0],CLUSTERID);

						if(subdataarr[0]->valueint == 1)
						{
							cJSON *sublearnmode = NULL;
							subctrl = cJSON_GetObjectItem(subarrayarr[0],"ctrl");
							if(subctrl == NULL) goto exit;
	
							subcmd  = cJSON_GetObjectItem(subctrl,"cmd");
							if(subcmd == NULL) goto exit;
						
							sublearnmode  = cJSON_GetObjectItem(subctrl,"learnmode");
							if(sublearnmode == NULL) goto exit;
						
							if(sublearnmode->model != cJSON_Number) goto exit;
						
							if(sublearnmode->valueint == 1)//学习模式
							{
								if(subcmd->model == cJSON_Number)
								{
									if(subcmd->valueint==10)//跟php服务器一致 10是触发
									{
										subfunctions  = cJSON_GetObjectItem(subctrl,"functions");
										
										if(subfunctions == NULL) goto exit;
								
										if(subfunctions->model != cJSON_Number) goto exit;
						
										if(subfunctions->valueint == 0)//发射
										{
//													uint8_t i=0;
											subposition  = cJSON_GetObjectItem(subctrl,"position");
											if(subposition == NULL) goto exit;
											if(cJSON_GetObjectItem(subctrl,"sw") != NULL) 
											{
	//										for(i =0;i<cJSON_GetObjectItem(subctrl,"times")->valueint;i++ )
												XK2233_sendLearnFun(subposition->valueint,cJSON_GetObjectItem(subctrl,"sw")->valueint);		
											}else
											{
												XK2233_sendLearnFun(subposition->valueint,123);
											}
										}else if(subfunctions->valueint == 1)//学习
										{
											subposition  = cJSON_GetObjectItem(subctrl,"position");
											if(subposition == NULL) goto exit;
											XK2233_learnFun(subposition->valueint);	
										}else
										{
											
										}	
									}	
								}
							}else if(sublearnmode->valueint == 0)//空调模式
							{
								//do it rightnow
								if(subcmd->model == cJSON_Number)
								{
									if(subcmd->valueint==10)//跟php服务器一致 10是触发
									{							
										if(cJSON_GetObjectItem(subctrl,JSONSTR_model) != NULL) 
										{
											XK2233_set_ACmodel(cJSON_GetObjectItem(subctrl,JSONSTR_model)->valueint);
										}
										if(cJSON_GetObjectItem(subctrl,"sw") != NULL) 
										{
											XK2233_ACsw(cJSON_GetObjectItem(subctrl,"sw")->valueint);
										}
										if(cJSON_GetObjectItem(subctrl,"mode") != NULL) 
										{
											XK2233_ACmode(cJSON_GetObjectItem(subctrl,"mode")->valueint);
										}
										if(cJSON_GetObjectItem(subctrl,"temp") != NULL) 
										{
											XK2233_sendACtemp(cJSON_GetObjectItem(subctrl,"temp")->valueint);
										}
										if(cJSON_GetObjectItem(subctrl,"speed") != NULL) 
										{
											XK2233_sendACspeed(cJSON_GetObjectItem(subctrl,"speed")->valueint);
										}
										if(cJSON_GetObjectItem(subctrl,"wdir") != NULL) 
										{
											XK2233_sendACWDIR(cJSON_GetObjectItem(subctrl,"wdir")->valueint);
										}
										if(0)
										{
											char chipbuffer[1];
											chipbuffer[0]=0x89;
											
											XK2233UART_RxWrite((uint8_t *)chipbuffer,sizeof(chipbuffer));
										}			
									}	
								}
							}else
							{
								
							}
						}			
					}
				}
			}
		}
	}
	exit:	delcJsonCreateObject();
}
void createXK2233StatusJson(XK2233_DATA data,char *messagemodel,char *status,uint16_t pid,uint16_t qos)
{
	char topic[256] = {""}; 
	memset(topic,'\0',sizeof(topic));
		if(1)
		{		
			cJsonCreateObject();
			cJSON_AddStringToObject(subinfoarr[0],JSONSTR_STATUS,ONLINE);
			
				cJSON_AddNumberToObject(subdataarr[0],"cmdstatus",data.cmdstatus);
				if(data.cmdstatus)
				{
					cJSON_AddStringToObject(subdataarr[0],"cmdstatusstr","success");
				}else
				{
					cJSON_AddStringToObject(subdataarr[0],"cmdstatusstr","failure");
				}
				cJSON_AddNumberToObject(subdataarr[0],"ircmd",data.ircmd);
				switch (data.ircmd)
				{
					case 0x88:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","learn mode");
						cJSON_AddNumberToObject(subdataarr[0],"code",data.code);
						break;
					case 0x86:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","send learned signal");
						cJSON_AddNumberToObject(subdataarr[0],"code",data.code);
					  if(data.sw == 0 || data.sw == 255)
					  cJSON_AddNumberToObject(subdataarr[0],"sw",data.sw);
						break;
					case 0x02:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","set AC model");
						cJSON_AddNumberToObject(subdataarr[0],JSONSTR_model,data.model);
						break;
					case 0x04:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","set ON/OFF");
						cJSON_AddNumberToObject(subdataarr[0],"sw",data.sw);
						break;
					case 0x05:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","set AC mode");
					cJSON_AddNumberToObject(subdataarr[0],"mode",data.mode);
						break;
					case 0x06:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","set AC temperature");
					cJSON_AddNumberToObject(subdataarr[0],"temp",data.temp);
						break;
					case 0x07:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","set AC speed");
					cJSON_AddNumberToObject(subdataarr[0],"speed",data.speed);
						break;
					case 0x08:
						cJSON_AddStringToObject(subdataarr[0],"ircmdstr","set AC winddir");
					cJSON_AddNumberToObject(subdataarr[0],"wdir",data.wdir);
						break;
					case 0xFF:
						printf("ircmd = %d\r\n",data.ircmd);
						break;
					default:
						printf("ircmd = %d\r\n",data.ircmd);
					return;
//					break;
				}
			//subdataarr[0]

			//subinfoarr[0]
			cJSON_AddStringToObject(subinfoarr[0],JSONSTR_model,"XK2233");
//			cJSON_AddStringToObject(subinfoarr[0],SN,"XK22330000000001");
			//添加到subrootarr[0]
			cJSON_AddItemToObject(subrootarr[0],JSONSTR_DATA,subdataarr[0]);
			cJSON_AddItemToObject(subrootarr[0],JSONSTR_INFO,subinfoarr[0]);
			cJSON_AddNumberToObject(subrootarr[0],CLUSTERID,1);
			cJSON_AddItemToArray(subarrayarr[0],subrootarr[0]);		
			/////////////////////////////////////////////////////////////////
		
			cJSON_AddStringToObject(root,"messagemodel",messagemodel);
			cJSON_AddNumberToObject(root,"messpacketid",pid);
			addRootInfo(status);
			//添加到root

			cJSON_AddItemToObject(root,PAYLOAD,subarrayarr[0]);

			cJSON_AddNumberToObject(root,DEVICEID,IRDeviceID);
			cJSON_AddStringToObject(root,SN,IRDEVICESN);
			cJSON_AddStringToObject(root,DEVICEmodel,IRDEVICEmodel);
			cJSON_AddStringToObject(root,DEVICEmodelSTR,IRDEVICEmodelSTR);	
			cJSON_AddNumberToObject(root,CLUSTERNUM,cJSON_GetArraySize(subarrayarr[0]));
			
//		strcat(topic,TOPIC_DATA);
//		strcat(topic,IRDeviceID);
			{
				char str_mac[13];
				{//4个初始值
					
					bsp_ChipSerialToMACStr(str_mac);
				}

			sprintf(topic,"data/%d/%s/%d",UI_ID,str_mac,IRDeviceID);	


			}
//		strcat(topic,"/data");
			printfJson(topic,qos,UNRETAINED,1);//一般是0
			
			delcJsonCreateObject();
		}
}
*/
