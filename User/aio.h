#ifndef AIO_H__
#define AIO_H__ 1

#include "main.h"

/** @defgroup AI_DPS_Variable AI 및 DPS 변수
  * @{
  */
 
#define AI_SENSING_AVERAGE_CNT 50                           /*!< Analog Input 평균 센싱 횟수 */
#define DP_SENSING_AVERAGE_CNT 50                           /*!< 압력센서 평균 센싱 횟수 */
#define DP_TEMPERATURE_ERROR   5                            /*!< 압력센서 에러 보상계수 0 ~ 10, 기준 5*/
#define DP_ADC_MINIMUM_VALUE   130                          /*!< DP_TEMPERATURE_ERROR 기본 값이 변경 될 경우 수정해야함.*/
#define DP_VFSS ((DP_TEMPERATURE_ERROR - 5) * 1 * 0.09 * 5) /*!< 0~80도에서 에러율 보정 값 */


/**
  * @}
  */

void AIO_Init(void);
uint16_t AI_Read(int8_t No);
void ADCStart(void);

uint16_t DP_Read(void);

#endif /* AIO_H__ */
