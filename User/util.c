/**
  ******************************************************************************
  * @file    util.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   유틸리티 함수
  * @details

  */

#include "util.h"
#include "iwdg.h"


void ADN_msleep(unsigned int ms)
{
  uint32_t CT, NT, ST;

  ST = HAL_GetTick();
  NT = ST + ms;

  while (1)
  {
    CT = HAL_GetTick();
    if (CT >= NT)
      break;
    if (CT - ST > 500) // 500 msec
    {
      ST = CT;
      HAL_IWDG_Refresh(&hiwdg);
    }
  }
}
