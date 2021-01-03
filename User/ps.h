#ifndef PS_H__
#define PS_H__ 1

#include "main.h"

/** @defgroup PS_Variable DIO 변수
  * @{
  */

#define PS_SENSING_AVERAGE_CNT 50 /*!< 차압센서 평균 센싱 횟수 */

/**
  * @}
  */
 
#define PS_ADDRESS  0x28U
#define PS_OUTPUT_MAX  14745 /*!> 14745 counts (90% of 2^14 counts or 0x3999) */
#define PS_OUTPUT_MIN  1638 /*!> 1638 counts (10% of 214 counts or 0x0666) */
#define PS_FORCE_RATED 10 /*!> maximum value of force range (N, lb, g, or kg) */

extern float psTemperature;

uint16_t PS_Read(void);
void PSStart(void);

#endif /* PS_H__ */
