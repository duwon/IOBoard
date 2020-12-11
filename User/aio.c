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

#define AI_SENSING_AVERAGE_CNT 100
#define PS_SENSING_AVERAGE_CNT 100

uint16_t ADCAiValue[2][AI_SENSING_AVERAGE_CNT];
uint16_t ADCPsValue[PS_SENSING_AVERAGE_CNT];
static uint32_t AISum[2];
static uint32_t PSSum;

static bool flag_ADCDone = false;
static uint16_t ADCValue[3];

/**
 * @brief Analog Input 
 * 
 * @param No : 포트 번호 : 1, 2
 * @return float : 전류 값
 */
float AI_Read(int8_t No)
{
  float analogValue = 0.0f;
  if (No < 2)
  {
    analogValue = (float)(AISum[No] / AI_SENSING_AVERAGE_CNT) * 1000 * 3.3f / 4096.0f / 20.0f;
    memset((void *)ADCAiValue[No], 0, sizeof(ADCAiValue[No]));
    AISum[No] = 0;
    }
  return analogValue;
}

float PS_Read(void)
{
  float analogValue = 0.0f;
  analogValue = (float)(PSSum / AI_SENSING_AVERAGE_CNT) * 3.3f / 4096.0f;
  memset((void *)ADCPsValue, 0, sizeof(ADCPsValue));
  PSSum = 0;
  return analogValue;
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
 * @brief AI 및 PS의 센싱 값 저장
 * 
 */
void ADCStart(void)
{
  static uint8_t AIAverageCnt = 0;
  static uint8_t PSAverageCnt = 0;

  if (flag_ADCDone == true)
  {
    flag_ADCDone = false;

    ADCAiValue[0][AIAverageCnt] = ADCValue[0];
    ADCAiValue[1][AIAverageCnt] = ADCValue[1];
    ADCPsValue[PSAverageCnt] = ADCValue[2];

    AISum[0] += ADCAiValue[0][AIAverageCnt];
    AISum[1] += ADCAiValue[1][AIAverageCnt++];
    PSSum += ADCPsValue[PSAverageCnt++];

    if (AIAverageCnt >= AI_SENSING_AVERAGE_CNT)
    {
      AIAverageCnt = 0;
    }
    if (PSAverageCnt >= PS_SENSING_AVERAGE_CNT)
    {
      PSAverageCnt = 0;
    }

    AISum[0] -= ADCAiValue[0][AIAverageCnt];
    AISum[1] -= ADCAiValue[1][AIAverageCnt];
    PSSum -= ADCPsValue[PSAverageCnt];
  }

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCValue, 3); /* ADC 시작 */
}
