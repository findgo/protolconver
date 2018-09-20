#include "app_cfg.h"

//for driver
#include "usart.h"
#include "systick.h"
#include "dlinkzigbee.h"

#include "memalloc.h"
#include "timers.h"
#include "event_groups.h"

#include "ltl.h"
#include "ltl_genattr.h"
#include "prefix.h"


static void prvClockInit(void);
static void prvnvicInit(void);
static TimerStatic_t tmstatic;
static TimerHandle_t tmhandle = NULL;
static void tmCb(void *arg);

static TimerStatic_t tmstaticF;
static TimerHandle_t tmhandleF = NULL;
static void tmCbF(void *arg);

int main(void)
{   
    prvClockInit();
    prvnvicInit();
    Systick_Configuration();
    Usart_Configuration();
    ltl_GeneralAttributeInit();

    delay_ms(200);
    dl_registerParseCallBack(NULL, ltlApduParsing);
    dlink_init();

    tmhandle = timerAssign(&tmstatic, tmCb,(void *)&tmhandle);
    timerStart(tmhandle, 1000);
    tmhandleF = timerAssign(&tmstaticF, tmCbF,(void *)&tmhandleF);
    timerStart(tmhandleF, 500);

    while(1)
    {
        dlinkTask();
        timerTask();
    }
//Should never reach this point!
}

static void tmCb(void *arg)
{
    uint16_t dst_addr = DL_BROADCAST_ADD;

    ltlReport_t *lreport;
    ltlAttrRec_t attrirecord;
    ltlReportCmd_t *reportcmd;

    if(ltlFindAttrRec(LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, ATTRID_BASIC_SERIAL_NUMBER, &attrirecord)){
 
        reportcmd = (ltlReportCmd_t *)mo_malloc(sizeof(ltlReportCmd_t) + sizeof(ltlReport_t) * 1);
        reportcmd->numAttr = 1;
        lreport = &(reportcmd->attrList[0]);
        lreport->attrID = attrirecord.attrId;
        lreport->dataType = attrirecord.dataType;
        lreport->attrData = attrirecord.dataPtr;

        ltl_SendReportCmd(&dst_addr, LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, 0, 
                        LTL_FRAMECTL_DIR_CLIENT_SERVER, LTL_MANU_CODE_SPECIFIC_LTL, TRUE,reportcmd);
        
        mo_free(reportcmd);
    }
    timerRestart(*((TimerHandle_t *)arg), 300);
}

static void tmCbF(void *arg)
{
    uint16_t dst_addr = DL_BROADCAST_ADD;

    ltlReport_t *lreport;
    ltlAttrRec_t attrirecord;
    ltlReportCmd_t *reportcmd;

    if(ltlFindAttrRec(LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, ATTRID_BASIC_MANUFACTURER_NAME, &attrirecord)){
 
        reportcmd = (ltlReportCmd_t *)mo_malloc(sizeof(ltlReportCmd_t) + sizeof(ltlReport_t) * 1);
        reportcmd->numAttr = 1;
        lreport = &(reportcmd->attrList[0]);
        lreport->attrID = attrirecord.attrId;
        lreport->dataType = attrirecord.dataType;
        lreport->attrData = attrirecord.dataPtr;

        ltl_SendReportCmd(&dst_addr, LTL_TRUNK_ID_GENERAL_BASIC, LTL_DEVICE_COMMON_NODENO, 0, 
                        LTL_FRAMECTL_DIR_CLIENT_SERVER, LTL_MANU_CODE_SPECIFIC_LTL, TRUE,reportcmd);
        
        mo_free(reportcmd);
    }
    timerRestart(*((TimerHandle_t *)arg), 500);
}

/*
 *=============================================================================
 *						  System Clock Configuration
 *=============================================================================
 *		 System Clock source		  | PLL(HSE)
 *-----------------------------------------------------------------------------
 *		 SYSCLK 					  | 72000000 Hz
 *-----------------------------------------------------------------------------
 *		 HCLK					  | 72000000 Hz
 *-----------------------------------------------------------------------------
 *		 PCLK1					  | 36000000 Hz
 *-----------------------------------------------------------------------------
 *		 PCLK2					  | 72000000 Hz
 *-----------------------------------------------------------------------------
 *		 ADCCLK					  | 12000000 Hz
 *-----------------------------------------------------------------------------
 *		 AHB Prescaler			  | 1
 *-----------------------------------------------------------------------------
 *		 APB1 Prescaler 			  | 2
 *-----------------------------------------------------------------------------
 *		 APB2 Prescaler 			  | 1
 *-----------------------------------------------------------------------------
 *		 ADC Prescaler 			  | 6
 *-----------------------------------------------------------------------------
 *		 HSE Frequency			  | 8000000 Hz
 *-----------------------------------------------------------------------------
 *		 PLL MUL					  | 9
 *-----------------------------------------------------------------------------
 *		 VDD						  | 3.3 V
 *-----------------------------------------------------------------------------
 *		 Flash Latency			  | 2 WS
 *-----------------------------------------------------------------------------
 *=============================================================================
*/
//Clock configuration
static void prvClockInit(void)
{
  ErrorStatus HSEStartUpStatus;

  RCC_DeInit();
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if (HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);

    /* AHB = DIV1 , HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* APB2 = DIV1, PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* APB1 = DIV2, PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* ADCCLK = PCLK2/6 = 72 / 6 = 12 MHz*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08);

    SystemCoreClockUpdate();
  } else {
    //Cannot start xtal oscillator!
    while(1); 
  }
}

//nvic configuration
static void prvnvicInit(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}




