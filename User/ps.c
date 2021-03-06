/**
  ******************************************************************************
  * @file    ps.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   압력센서, 차압센서 제어
  * @details

  */

#include "ps.h"
#include "i2c.h"
#include "adc.h"
#include <stdio.h>
#include "string.h"

/** @defgroup PS 압력센서 제어 함수
  * @brief 압력센서(PS) 제어
  * @{
  */

/** @defgroup PS_Variable 압력센서 변수
  * @{
  */

float psTemperature = 0U;						 /*!> 온도 : 0.1도 해상도 */
static uint16_t PsValue[PS_SENSING_AVERAGE_CNT]; /*!< 압력센서 값 저장 버퍼 */
static uint32_t PSSum;							 /*!< 압력센서 합 */

/**
  * @}
  */

/**
 * @brief 압력센서 평균 센싱값 반환
 * 
 * @return uint16_t: 압력센서 값 * 100
 */
uint16_t PS_Read(void)
{
	uint16_t pressValue = 0U;
	pressValue = PSSum / PS_SENSING_AVERAGE_CNT;
	memset((void *)PsValue, 0, sizeof(PsValue));
	PSSum = 0;
	return pressValue;
}

/**
 * @brief 압력센서 1회 센싱 수행
 * 
 */
void PSStart(void)
{
	static uint8_t PSAverageCnt = 0;
	float pressValue;
	uint8_t psState = 1U; /*!> 상태 */
	uint16_t psBar = 0U;  /*!> 압력  */

	uint8_t i2cRxData[4] = {
		0xFFU,
	};
	HAL_I2C_Master_Receive(&hi2c2, (0x28 << 1) + 0x01, i2cRxData, 4, 0xF);

	psState = (i2cRxData[0] >> 6U) & 0x03U;
	psBar = ((i2cRxData[0] & 0x3FU) << 8U) + i2cRxData[1];
	psTemperature = (float)((i2cRxData[2] << 3U) + ((i2cRxData[3] >> 5U) & 0x07U)) * 200 / 2047 - 50;

	if (psState == 0)
	{
		pressValue = (float)((psBar - PS_OUTPUT_MIN) * PS_FORCE_RATED) / (float)(PS_OUTPUT_MAX - PS_OUTPUT_MIN);

		PsValue[PSAverageCnt] = (uint16_t)((pressValue - 1) * 1000);

		PSSum += PsValue[PSAverageCnt++];

		if (PSAverageCnt >= PS_SENSING_AVERAGE_CNT)
		{
			PSAverageCnt = 0;
		}

		PSSum -= PsValue[PSAverageCnt];
	}
}

/**
  * @}
  */
