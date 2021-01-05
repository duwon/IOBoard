#ifndef RTD_H__
#define RTD_H__ 1

#include "main.h"

/** @defgroup RTD_Variable RTD 변수
  * @{
  */
 
#define RTD_SENSING_AVERAGE_CNT 20 /*!< RTD 센서의 평균 셍싱 횟수 */

/**
  * @}
  */

void RTD_Init(void);
void RTDSTart(void); /*!< RTD 평균을 위한 센싱 시작 함수 */
int16_t RTD_Read(void);

void LMP90080_GPIO_Write(uint8_t PinNum, uint8_t PinState);

#endif /* RTD_H__ */
