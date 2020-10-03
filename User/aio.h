#ifndef AIO_H__
#define AIO_H__ 1

#include "main.h"

#define ADC_COUNT 4U /*!< ADC 갯수 */
#define ADC_SAMPLE_1S_COUNT 100U /*!< Timer8(1초) 이벤트 동안 ADC 샘플 갯수 */
#define ADC_SAMPLE_COUNT (ADC_SAMPLE_1S_COUNT * ADC_COUNT) /*!< 샘플링 갯수 */


typedef enum 
{
  DAC_NORMAL = 0,
  DAC_1K,
  DAC_100K,
  DAC_HIGH_Z
} DAC_MODE_TypeDef;

void AI_Init(void);
void AO_Init(void);
float AI_Read(int8_t No);
void AIN_TIM_PeriodElapsedCallback(void);

#endif /* AIO_H__ */
