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
#include "aio.h"

bool flag_1SecTimerOn = false;       /*!< 1s Flag */
bool flag_1mSecTimerOn = false;      /*!< 1ms Flag */
bool flag_10mSecTimerOn = false;     /*!< 10ms Flag */
bool flag_sendStatusTimerOn = false; /*< 1초 마다 상태 전송 Flag */

/**
  * @brief  Timer 인터럽트 함수. 이 함수는 HAL_TIM_IRQHandler() 내부에서 TIM6, TIM7 인터럽트가 발생했을 때 호출됨.
  * @note   TIM5 - 1s Timer, TIM6 - 1ms Timer, TIM7 - 10ms Timer
  * 
  * @param  htim : TIM handle
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint32_t count_sendStatus = 0U;

  if (htim->Instance == TIM7) /* 10ms - AI 인터럽트 호출 용*/
  {
	  flag_10mSecTimerOn = true;
  }
  else if (htim->Instance == TIM6) /* 1ms */
  {
    count_sendStatus++;
    if (count_sendStatus > SEND_STATUS_INTERVAL)
    {
      flag_sendStatusTimerOn = true;
      count_sendStatus = 0U;
    }

    flag_1mSecTimerOn = true;
  }
  else if (htim->Instance == TIM5) /* 1s */
  {
    flag_1SecTimerOn = true;
  }
}

/**
 * @brief 타이머 시작
 * 
 */
void Timer_Init(void)
{
  HAL_TIM_Base_Start_IT(&htim5); /* 1s 타이머 인터럽트 시작 */
  HAL_TIM_Base_Start_IT(&htim6); /* 1ms 타이머 인터럽트 시작 */
  HAL_TIM_Base_Start_IT(&htim7); /* 0.1ms 타이머 인터럽트 시작 */
}

/** @defgroup RTC RTC 제어 함수
  * @brief RTC 제어
  * @{
  */

/** @defgroup RTC_Variable RTC 변수
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
  RTC_TimeTypeDef sTempTime;
  RTC_DateTypeDef sTempDate;
  uint8_t I2CTxData[1] = {0x00U};
  uint8_t I2CRxData[7];

  HAL_I2C_Master_Transmit(&hi2c1, RTC_I2C_ADDRESS, I2CTxData, 1, 0xFFU);                /* 주소번지 0 전송 */
  HAL_I2C_Master_Receive(&hi2c1, RTC_I2C_ADDRESS, I2CRxData, sizeof(I2CRxData), 0xFFU); /* 7바이트 읽음 */

  sTempTime.Seconds = I2CRxData[0];
  sTempTime.Minutes = I2CRxData[1];
  sTempTime.Hours = I2CRxData[2];
  sTempDate.Date = I2CRxData[4];
  sTempDate.Month = I2CRxData[5];
  sTempDate.Year = I2CRxData[6];

  HAL_RTC_SetTime(&hrtc, &sTempTime, RTC_FORMAT_BCD);
  HAL_RTC_SetDate(&hrtc, &sTempDate, RTC_FORMAT_BCD);
}

/**
 * @brief 시간 얻기
 * 
 * @return uint8_t*: RTC_FORMAT_BIN 포맷의 포인터 [0:5] Y:M:D:H:M:M
 */
void RTC_Get(uint8_t *pTime)
{
  RTC_TimeTypeDef sTempTime;
  RTC_DateTypeDef sTempDate;

  HAL_RTC_GetTime(&hrtc, &sTempTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sTempDate, RTC_FORMAT_BIN);

  pTime[5] = sTempTime.Seconds;
  pTime[4] = sTempTime.Minutes;
  pTime[3] = sTempTime.Hours;
  pTime[2] = sTempDate.Date;
  pTime[1] = sTempDate.Month;
  pTime[0] = sTempDate.Year;
}

/**
 * @brief RTC에 시간 저장
 * 
 * @param pTime: RTC_FORMAT_BIN 포맷 포인터 [0:5] Y:M:D:H:M:M
 */
void RTC_Set(uint8_t *pTime)
{
  RTC_TimeTypeDef sTempTime;
  RTC_DateTypeDef sTempDate;

  sTempTime.Seconds = pTime[5];
  sTempTime.Minutes = pTime[4];
  sTempTime.Hours = pTime[3];
  sTempDate.Date = pTime[2];
  sTempDate.Month = pTime[1];
  sTempDate.Year = pTime[0];

  HAL_RTC_SetTime(&hrtc, &sTempTime, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, &sTempDate, RTC_FORMAT_BIN);

  HAL_RTC_GetTime(&hrtc, &sTempTime, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc, &sTempDate, RTC_FORMAT_BCD);

  uint8_t I2CTxData[8] = {
      0,
  };
  I2CTxData[0] = 0x00U; /* Register Address */
  I2CTxData[1] = sTempTime.Seconds;
  I2CTxData[2] = sTempTime.Minutes;
  I2CTxData[3] = sTempTime.Hours;
  I2CTxData[5] = sTempDate.Date;
  I2CTxData[6] = sTempDate.Month;
  I2CTxData[7] = sTempDate.Year;

  HAL_I2C_Master_Transmit(&hi2c1, RTC_I2C_ADDRESS, I2CTxData, sizeof(I2CTxData), 0xFFU);
}

void RTC_SaveValue(uint64_t saveValue)
{
  uint8_t I2CTxData[9] = {
    0,
  };

  I2CTxData[0] = 0x14U; /* Register Address */
  I2CTxData[1] = (saveValue >> 56) & 0xFF;
  I2CTxData[2] = (saveValue >> 48) & 0xFF;
  I2CTxData[3] = (saveValue >> 40) & 0xFF;
  I2CTxData[4] = (saveValue >> 32) & 0xFF;
  I2CTxData[5] = (saveValue >> 24) & 0xFF;
  I2CTxData[6] = (saveValue >> 16) & 0xFF;
  I2CTxData[7] = (saveValue >> 8) & 0xFF;
  I2CTxData[8] = (saveValue >> 0) & 0xFF;

  HAL_I2C_Master_Transmit(&hi2c1, RTC_I2C_ADDRESS, I2CTxData, sizeof(I2CTxData), 0xFFU);
}

uint64_t RTC_LoadValue(void)
{
  uint8_t I2CTxData[1] = {0x14U};
  uint8_t I2CRxData[8];
  uint64_t returnValue;

  HAL_I2C_Master_Transmit(&hi2c1, RTC_I2C_ADDRESS, I2CTxData, 1, 0xFFU);                /* 주소번지 0 전송 */
  HAL_I2C_Master_Receive(&hi2c1, RTC_I2C_ADDRESS, I2CRxData, sizeof(I2CRxData), 0xFFU); /* 8바이트 읽음 */

  returnValue  = (uint64_t)I2CRxData[0] << 56;
  returnValue += (uint64_t)I2CRxData[1] << 48;
  returnValue += (uint64_t)I2CRxData[2] << 40;
  returnValue += (uint64_t)I2CRxData[3] << 32;
  returnValue += (uint64_t)I2CRxData[4] << 24;
  returnValue += (uint64_t)I2CRxData[5] << 16;
  returnValue += (uint64_t)I2CRxData[6] << 8 ;
  returnValue += (uint64_t)I2CRxData[7] << 0 ;

  return returnValue;
}
/**
  * @}
  */
