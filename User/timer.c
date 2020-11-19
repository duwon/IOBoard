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
bool flag_100uSecTimerOn = false;    /*!< 100us Flag */
bool flag_sendStatusTimerOn = false; /*< 1초 마다 상태 전송 Flag */
/**
  * @brief  Timer 인터럽트 함수. 이 함수는 HAL_TIM_IRQHandler() 내부에서 TIM6, TIM7 인터럽트가 발생했을 때 호출됨.
  * @note   TIM6 - 1ms Timer, TIM7 - 0.1ms Timer
  * 
  * @param  htim : TIM handle
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  static uint32_t count_1s = 0;
  static uint32_t count_sendStatus = 0;

  if (htim->Instance == TIM6) /* 1ms */
  {
    count_1s++;
    if (count_1s > 1000)
    {
      flag_1SecTimerOn = true;
      count_1s = 0;
    }

    count_sendStatus++;
    if (count_sendStatus > SEND_STATUS_INTERVAL)
    {
      flag_sendStatusTimerOn = true;
      count_sendStatus = 0;
    }

    flag_1mSecTimerOn = true;
  }
  if (htim->Instance == TIM7) /* 0.1ms - AI 인터럽트 호출*/
  {
    flag_100uSecTimerOn = true;
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
  RTC_TimeTypeDef sTempTime;
  RTC_DateTypeDef sTempDate;
  uint8_t I2CTxData[1] = {0x00U};
  uint8_t I2CRxData[7];

  HAL_I2C_Master_Transmit(&hi2c1, RTC_I2C_ADDRESS, I2CTxData, 1, 0xFFU);                /* 주소번지 0 전송 */
  HAL_I2C_Master_Receive(&hi2c1, RTC_I2C_ADDRESS, I2CRxData, sizeof(I2CRxData), 0xFFU); /* 6바이트 읽음 */

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

/**
  * @}
  */
