/**
  ******************************************************************************
  * @file    dp.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   압력센서, 차압센서 제어
  * @details

  */

#include "dp.h"
#include "i2c.h"
#include "adc.h"
#include <stdio.h>
#include "string.h"

#define DP_SENSING_AVERAGE_CNT 50 /*!< 차압센서 평균 센싱 횟수 */

uint16_t DpValue[DP_SENSING_AVERAGE_CNT];
static uint32_t DPSum;

/**
 * @brief 압력센서 평균 센싱값 반환
 * 
 * @return uint16_t: 압력센서 값 * 100
 */
uint16_t DP_Read(void)
{
	  uint16_t pressValue = 0U;
	  pressValue = DPSum / DP_SENSING_AVERAGE_CNT;
	  memset((void *)DpValue, 0, sizeof(DpValue));
	  DPSum = 0;
	  return pressValue;
}

float dpTemperature = 0U; /*!> 온도 : 0.1도 해상도 */
/**
 * @brief 압력센서 1회 센싱 수행
 * 
 */
void DPStart(void)
{
	static uint8_t DPAverageCnt = 0;
	float pressValue;
	uint8_t dpState = 1U; /*!> 상태 */
	uint16_t dpBar = 0U; /*!> 압력  */

	uint8_t i2cRxData[4] = {0xFFU,};
	HAL_I2C_Master_Receive(&hi2c2, (0x28 << 1) + 0x01 , i2cRxData, 4, 0xF);

	dpState = (i2cRxData[0] >> 6U) & 0x03U;
	dpBar = ((i2cRxData[0] & 0x3FU) << 8U) + i2cRxData[1];
	dpTemperature = (float)((i2cRxData[2] << 3U) + ((i2cRxData[3] >> 5U) & 0x07U)) * 200 / 2047 - 50;

	if(dpState == 0)
	{
		pressValue = (float)((dpBar - DP_OUTPUT_MIN) * DP_FORCE_RATED ) / (float)(DP_OUTPUT_MAX - DP_OUTPUT_MIN);

	    DpValue[DPAverageCnt] = (uint16_t)((pressValue - 1)  * 100);

	    DPSum += DpValue[DPAverageCnt++];

	    if (DPAverageCnt >= DP_SENSING_AVERAGE_CNT)
	    {
	    	DPAverageCnt = 0;
	    }

	    DPSum -= DpValue[DPAverageCnt];
	}
}

