

#include "hal_led.h"




void halledInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_LED_GPIO_PeriphClock_EN();

//for red led
	GPIO_InitStruct.GPIO_Pin = HAL_LED_BLUE_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(HAL_LED_BLUE_PORT, &GPIO_InitStruct);
}
