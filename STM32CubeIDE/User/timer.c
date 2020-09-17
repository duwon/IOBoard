/**
  ******************************************************************************
  * @file    timer.c
  * @author  정두원
  * @date    2020-04-10
  * @brief   타이머 제어
  * @details

  */

#include "timer.h"
#include "tim.h"

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
}