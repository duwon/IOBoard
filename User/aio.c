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

#define AI_SENSING_AVERAGE_CNT 50
#define PS_SENSING_AVERAGE_CNT 50

uint16_t AiValue[2][AI_SENSING_AVERAGE_CNT];
uint16_t PsValue[PS_SENSING_AVERAGE_CNT];
static uint32_t AISum[2];
static uint32_t PSSum;

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
    //analogValue = (float)(AISum[No] / AI_SENSING_AVERAGE_CNT) * 1000 * VrefVoltage / 4096.0f / 20.0f; /* I = V/R = (평균 ADC값 * mA(1000) * Vref / 분해능) / 저항 값 */
	analogValue = AISum[No] / AI_SENSING_AVERAGE_CNT;
    memset((void *)AiValue[No], 0, sizeof(AiValue[No]));
    AISum[No] = 0;
    }
  return analogValue;
}

uint16_t PS_Read(void)
{
  uint16_t calValue = 0U;
  calValue = (uint16_t)((float)((PSSum / AI_SENSING_AVERAGE_CNT) - 10) * 4.444f);
  memset((void *)PsValue, 0, sizeof(PsValue));
  PSSum = 0;
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
 * @brief AI 및 PS의 센싱 값 저장
 * 
 */
void ADCStart(void)
{
  static uint8_t AIAverageCnt = 0;
  static uint8_t PSAverageCnt = 0;

  if (flag_ADCDone == true)
  {
	float Vref = 1.2f * 4096.0f / (float)(ADCValue[3]);
    flag_ADCDone = false;
    AiValue[0][AIAverageCnt] = (uint16_t)((float)(ADCValue[0]) * Vref * 1000 * 100 / 4096.0f / 20.0f);
    AiValue[1][AIAverageCnt] = (uint16_t)((float)(ADCValue[1]) * Vref * 1000 * 100 / 4096.0f / 20.0f);
    PsValue[PSAverageCnt] = (uint16_t)((float)ADCValue[2] * Vref * 100 /4096.0f);

    AISum[0] += AiValue[0][AIAverageCnt];
    AISum[1] += AiValue[1][AIAverageCnt++];
    PSSum += PsValue[PSAverageCnt++];

    if (AIAverageCnt >= AI_SENSING_AVERAGE_CNT)
    {
      AIAverageCnt = 0;
    }
    if (PSAverageCnt >= PS_SENSING_AVERAGE_CNT)
    {
      PSAverageCnt = 0;
    }


    AISum[0] -= AiValue[0][AIAverageCnt];
    AISum[1] -= AiValue[1][AIAverageCnt];
    PSSum -= PsValue[PSAverageCnt];
  }

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADCValue, 4); /* ADC 시작 */
}
