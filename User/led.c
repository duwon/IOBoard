/**
  ******************************************************************************
  * @file    led.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   LED 제어
  * @details

  */

#include "led.h"


/** @defgroup LED LED 제어 함수
  * @brief 지정된 LED 파라미터의 제어
  * @{
  */

/** @defgroup LED_Variable LED 변수
  * @{
  */ 
#define LEDPORTn  11 /*!< LED 포트 갯수 */
GPIO_TypeDef *LED_PORT[LEDPORTn] = {LD_DI1_GPIO_Port, LD_DI2_GPIO_Port, LD_DI3_GPIO_Port, LD_DI4_GPIO_Port, LD_AI1_GPIO_Port, LD_AI2_GPIO_Port, LD_RS485RDY_GPIO_Port, LD_RS232RDY_GPIO_Port, LD_DO1_GPIO_Port, LD_DO2_GPIO_Port, LD_AO_GPIO_Port};
const uint16_t LED_PIN[LEDPORTn] = {LD_DI1_Pin, LD_DI2_Pin, LD_DI3_Pin, LD_DI4_Pin, LD_AI1_Pin, LD_AI2_Pin, LD_RS485RDY_Pin, LD_RS232RDY_Pin, LD_DO1_Pin, LD_DO2_Pin, LD_AO_Pin};
/**
  * @}
  */ 


/**
  * @brief  선택한 LED On.
  * @param  Led: 다음의 파라미터만 입력:
  *            @arg  LD_DI1, LD_DI2, LD_DI3, LD_DI4, LD_AI1, LD_AI2, LD_RS485RDY, LD_RS232RDY, LD_DO1, LD_DO2, LD_AO
  */
void LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

/**
  * @brief  선택한 LED Off. 
  * @param  Led: 다음의 파라미터만 입력:
  *            @arg  LD_DI1, LD_DI2, LD_DI3, LD_DI4, LD_AI1, LD_AI2, LD_RS485RDY, LD_RS232RDY, LD_DO1, LD_DO2, LD_AO
  */
void LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET); 
}

/**
  * @brief  선택한 LED Toggle. 
  * @param  Led: 다음의 파라미터만 입력:
  *            @arg  LD_DI1, LD_DI2, LD_DI3, LD_DI4, LD_AI1, LD_AI2, LD_RS485RDY, LD_RS232RDY, LD_DO1, LD_DO2, LD_AO
  * @retval None
  */
void LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}

/**
  * @}
  */
