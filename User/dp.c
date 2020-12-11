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

float DP_Read(void)
{
	float pressValue;
	  uint8_t dpState = 1U; /*!> 상태 */
	  uint16_t dpBar = 0U; /*!> 압력  */
	  float dpTemperature = 0U; /*!> 온도 : 0.1도 해상도 */

	  uint8_t i2cRxData[4] = {0xFFU,};
	  HAL_I2C_Master_Receive(&hi2c2, (0x28 << 1) + 0x01 , i2cRxData, 4, 0xF);

	  dpState = (i2cRxData[0] >> 6U) & 0x03U;
	  dpBar = ((i2cRxData[0] & 0x3FU) << 8U) + i2cRxData[1];
	  dpTemperature = (float)((i2cRxData[2] << 3U) + ((i2cRxData[3] >> 5U) & 0x07U)) * 200 / 2047 - 50;

	  if(dpState == 0)
	  {
		  pressValue = (float)((dpBar - DP_OUTPUT_MIN) * DP_FORCE_RATED ) / (float)(DP_OUTPUT_MAX - DP_OUTPUT_MIN);
	  }

	return pressValue;
}

void PSENOR_Read(void)
{

}
