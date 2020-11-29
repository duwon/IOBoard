/**
  ******************************************************************************
  * @file    dio.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   Digital Input/Output 제어
  * @details

  */

#include "dio.h"
#include "led.h"

/** @defgroup DIO Digital Input/Output 제어 함수
  * @brief Digital I/O 제어
  * @{
  */

/** @defgroup DIO_Variable DIO 변수
  * @{
  */
#define DIPORTn 4 /*!< digital input 포트 갯수 */
GPIO_TypeDef *DI_PORT[DIPORTn] = {DI1_GPIO_Port, DI2_GPIO_Port, DI3_GPIO_Port, DI4_GPIO_Port};
const uint16_t DI_PIN[DIPORTn] = {DI1_Pin, DI2_Pin, DI3_Pin, DI4_Pin};

#define DOPORTn 2                                                /*!< digital output 포트 갯수 */
GPIO_TypeDef *DO_PORT[DOPORTn] = {DO1_GPIO_Port, DO2_GPIO_Port}; /* DO 포트 */
const uint16_t DO_PIN[DOPORTn] = {DO1_Pin, DO2_Pin};

GPIO_TypeDef *LD_DO_PORT[DOPORTn] = {LD_DO1_GPIO_Port, LD_DO2_GPIO_Port}; /* LED DO 포트 */
const uint16_t LD_DO_PIN[DOPORTn] = {LD_DO1_Pin, LD_DO2_Pin};

/**
  * @}
  */

/**
 * @brief Digital Output 포트에 값을 출력
 * 
 * @param No: 포트 번호
 *          @arg 0, 1
 * @param Value: 출력 값
 *          @arg 0 : OFF, 1 : ON
 */
void DO_Write(int8_t No, int8_t Value)
{
  if (No < DOPORTn)
  {
    HAL_GPIO_WritePin(DO_PORT[No], DO_PIN[No], (Value == 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(LD_DO_PORT[No], LD_DO_PIN[No], (Value == 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

/**
 * @brief 4개의 Digital Input 포트의 상태 읽기
 * 
 * @param returnValue: 포트 상태 값 : 0-OFF, 1-ON
 *          [3:0] : [Port4, Port3, Port2, Port1]
 */
void DI_Read(uint8_t *returnValue)
{

  for (uint8_t i = 0; i < DIPORTn; i++)
  {
    if (GPIO_PIN_SET == HAL_GPIO_ReadPin(DI_PORT[i], DI_PIN[i]))
    {
      returnValue[i] = 0;
      LED_Off(LD_DI1 + i);
    }
    else
    {
      returnValue[i] = 1;
      LED_On(LD_DI1 + i);
    }
  }
}

/**
  * @}
  */
