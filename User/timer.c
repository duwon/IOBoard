/**
  ******************************************************************************
  * @file    timer.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   타이머 제어
  * @details

  */

#include "timer.h"
#include "tim.h"

/**
  * @brief 1초 Flag
  */
bool flag_1SecTimerOn = false; /* 1초 Flag */

/**
  * @brief  Timer 인터럽트 함수. 이 함수는 HAL_TIM_IRQHandler() 내부에서 TIM6, TIM7 인터럽트가 발생했을 때 호출됨.
  * @note   TIM6 - 1ms Timer, TIM7 - 0.1ms Timer
  * 
  * @param  htim : TIM handle
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint32_t count_1s = 0;
  if (htim->Instance == TIM6) /* 1ms */
  {
    count_1s++;
    if( count_1s > 1000)
    {
      flag_1SecTimerOn = true;
    }
  }
  if (htim->Instance == TIM7) /* 0.1ms */
  {
  
  }
}


