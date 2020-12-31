/**
  ******************************************************************************
  * @file    aio.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   Analog Input 제어
  * @details

  */

#include <string.h>
#include <stdio.h>
#include "aio.h"
#include "uart.h"
#include "adc.h"
#include "uart.h"
#include "timer.h"

#define AI_SENSING_AVERAGE_CNT 50 /*!< Analog Input 평균 센싱 횟수 */
#define DP_SENSING_AVERAGE_CNT 50 /*!< 압력센서 평균 센싱 횟수 */
#define DP_TEMPERATURE_ERROR   5  /*!< 압력센서 에러 보상계수 0 ~ 10, 기준 5*/
#define DP_VFSS ((DP_TEMPERATURE_ERROR-5) * 1 * 0.09 * 5) /*!< 0~80도에서 에러율 보정 값 */

uint16_t AiValue[2][AI_SENSING_AVERAGE_CNT];
uint16_t DpValue[DP_SENSING_AVERAGE_CNT];
static uint32_t AISum[2];
static uint32_t DPSum;

static bool flag_ADCDone = false;
static uint16_t ADCValue[4];

/**
 * @brief Analog Input 
 * 
 * @param No : 포트 번호 : 1, 2
 * @return uint16_t : 전류 값 * 100
 */
uint16_t AI_Read(int8_t No)
{
  uint16_t analogValue = 0U;
  if (No < 2)
  {
    analogValue = AISum[No] / (AI_SENSING_AVERAGE_CNT - 1);
    memset((void *)AiValue[No], 0, sizeof(AiValue[No]));
    AISum[No] = 0;
  }
  return analogValue;
}

uint16_t DP_Read(void)
{
  uint16_t calValue = 0U;
  calValue = (uint16_t)((float)((DPSum / (DP_SENSING_AVERAGE_CNT -1)) - 200) * 0.224f + (float)DP_VFSS);
  memset((void *)DpValue, 0, sizeof(DpValue));
  DPSum = 0;
  return calValue;
}

void AIO_Init(void)
{
  HAL_ADCEx_Calibration_Start(&hadc1); /* ADC Calibration */
  HAL_Delay(1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCValue, 3); /* ADC 시작 */
}

/**
 * @brief ADC 완료 인터럽트
 * 
 * @param hadc 
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  flag_ADCDone = true;
}

/**
 * @brief AI 및 DP의 센싱 값 저장
 * 
 */
void ADCStart(void)
{
  static uint8_t AIAverageCnt = 0;
  static uint8_t DPAverageCnt = 0;

  if (flag_ADCDone == true)
  {
    float Vref = 1.2f * 4096.0f / (float)(ADCValue[3]); /* Vref 보정 값 */
    flag_ADCDone = false;
    /* AI0, AI1 실크가 바뀐듯 */
    AiValue[1][AIAverageCnt] = (uint16_t)((float)(ADCValue[0]) * Vref * 1000000 / 4096.0f / 20.0f); 
    AiValue[0][AIAverageCnt] = (uint16_t)((float)(ADCValue[1]) * Vref * 1000000 / 4096.0f / 20.0f); /* (ADC값 / 2^12) * Vref * 20옴 * uA */
    DpValue[DPAverageCnt] = (uint16_t)((float)ADCValue[2] * Vref * 0.488f); /* (ADC 값 / 2^12) * Vref * 1000 / 저항분배(0.5) */

    AISum[0] += AiValue[0][AIAverageCnt];
    AISum[1] += AiValue[1][AIAverageCnt++];
    DPSum += DpValue[DPAverageCnt++];

    if (AIAverageCnt >= AI_SENSING_AVERAGE_CNT)
    {
      AIAverageCnt = 0;
    }
    if (DPAverageCnt >= DP_SENSING_AVERAGE_CNT)
    {
      DPAverageCnt = 0;
    }

    AISum[0] -= AiValue[0][AIAverageCnt];
    AISum[1] -= AiValue[1][AIAverageCnt];
    DPSum -= DpValue[DPAverageCnt];
  }

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCValue, 4); /* ADC 시작 */
}
