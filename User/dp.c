/**
  ******************************************************************************
  * @file    dp.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   차압센서 제어
  * @details

  */

#include "dp.h"
#include "i2c.h"

float DP_Read(void)
{
	float dpValue;
  uint8_t dpState = 0; /*!> 상태 */
  uint16_t dpBar = 0; /*!> 압력  */
  uint16_t dpTemperature = 0; /*!> 온도 : 0.1도 해상도 */

  uint8_t i2cRxData[4] = {0U,};
  HAL_I2C_Master_Receive(&hi2c1, DP_ADDRESS, i2cRxData, 4, 0xFF);

  dpState = (i2cRxData[0] >> 6U) & 0x03U;
  dpBar = ((i2cRxData[0] & 0x3FU) << 8U) + i2cRxData[1];
  dpTemperature = (i2cRxData[2] << 3U) + (i2cRxData[3] >> 5U) & 0x07U;

  if(dpState == 0)
  {
    dpValue = (float)((dpBar - DP_OUTPUT_MIN) * DP_FORCE_RATED ) / (float)(DP_OUTPUT_MAX - DP_OUTPUT_MIN);
  }

	return dpValue;
}
