/**
  ******************************************************************************
  * @file    timer.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   타이머 제어 및 내외부 RTC 제어
  * @details

  */

#include "timer.h"
#include "tim.h"
#include "rtc.h"
#include "i2c.h"

bool flag_1SecTimerOn = false; /*!< 1초 Flag */

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

/**
 * @brief 
 * 
 */
void Timer_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim6); /* 1ms 타이머 인터럽트 시작 */
  HAL_TIM_Base_Start_IT(&htim7); /* 0.1ms 타이머 인터럽트 시작 */
}

/** @defgroup DIO Digital Input/Output 제어 함수
  * @brief Digital I/O 제어
  * @{
  */

/** @defgroup DIO_Variable DIO 변수
  * @{
  */ 
#define RTC_I2C_ADDRESS 0xD0U /*!< RTC IC ADDRESS */
/**
  * @}
  */ 

/**
 * @brief RTC IC로부터 시간 불러오기. 내부 RTC 설정
 * 
 */
void RTC_Load(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  uint8_t I2CTxData[1] = {0x00U};
  uint8_t I2CRxData[7];

  HAL_I2C_Master_Transmit(&hi2c2, RTC_I2C_ADDRESS, I2CTxData, 1, 0xFFU); /* 주소번지 0 전송 */
  HAL_I2C_Master_Receive(&hi2c2, RTC_I2C_ADDRESS, I2CRxData, sizeof(I2CRxData), 0xFFU); /* 6바이트 읽음 */

  sTime.Seconds = I2CRxData[0];
  sTime.Minutes = I2CRxData[1];
  sTime.Hours = I2CRxData[2];
  sDate.Date = I2CRxData[4];
  sDate.Month = I2CRxData[5];
  sDate.Year = I2CRxData[6] + 0x10; /* STM32는 1990년부터. DS3232는 2020년부터 */

  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
}

/**
 * @brief RTC에 시간 저장
 * 
 * @param sDate: RTC_DateTypeDef 구조체
 * @param sTime: RTC_TimeTypeDef 구조체
 * @param Format: sDate, sTime 구조체의 포맷.
 *            @arg RTC_FORMAT_BIN: Binary data format
 *            @arg RTC_FORMAT_BCD: BCD data format
 */
void RTC_Set(RTC_DateTypeDef sDate, RTC_TimeTypeDef sTime, uint32_t Format)
{
  RTC_TimeTypeDef sTempTime;
  RTC_DateTypeDef sTempDate;
  uint8_t I2CTxData[8];

  HAL_RTC_SetTime(&hrtc, &sTime, Format);
  HAL_RTC_SetDate(&hrtc, &sDate, Format);

  HAL_RTC_SetTime(&hrtc, &sTempTime, RTC_FORMAT_BCD);
  HAL_RTC_SetDate(&hrtc, &sTempDate, RTC_FORMAT_BCD);

  I2CTxData[0] = 0x00U; /* Register Address */
  I2CTxData[1] = sTime.Seconds;
  I2CTxData[2] = sTime.Minutes;
  I2CTxData[3] = sTime.Hours;
  I2CTxData[5] = sDate.Date;
  I2CTxData[6] = sDate.Month;
  I2CTxData[7] = sDate.Year - 0x10; /* STM32는 1990년부터. DS3232는 2020년부터 */

  HAL_I2C_Master_Transmit(&hi2c2, RTC_I2C_ADDRESS, I2CTxData, sizeof(I2CTxData), 0xFFU);

}

/**
  * @}
  */ 
