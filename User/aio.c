/**
  ******************************************************************************
  * @file    aio.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   Analog Input 제어
  * @details

  */

#include <string.h>
#include "aio.h"
#include "tim.h"
#include "spi.h"

void AIO_Init(void)
{
}

/**
 * @brief Analog Input 
 * 
 * @param No : 포트 번호 : 1, 2
 * @return float : 전류 값
 */
float AI_Read(int8_t No)
{
  float analogValue = 0.0f;
  return analogValue;
}

void LMP90080_WriteReg(uint8_t regNum, uint8_t regData)
{
}

uint16_t LMP90080_ReadReg(uint8_t regNum)
{

  return 0;
}

/**
 * @brief DAC 제어
 * 
 * @param OPMode: DAC 동작 모드
 *    @arg DAC_NORMAL, DAC_1K, DAC_100K, DAC_HIGH_Z
 * @param Data: DAC 값
 */
void DAC8551_WriteReg(DAC_MODE_TypeDef OPMode, uint16_t Data)
{
  uint8_t spiData[3] = {(uint8_t)OPMode, (uint8_t)((Data >> 8U) & 0xFFU), (uint8_t)(Data & 0xFFU)};
  HAL_SPI_Transmit_DMA(&hspi3, spiData, 3U);
}
