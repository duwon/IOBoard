#ifndef AIO_H__
#define AIO_H__ 1

#include "main.h"

#define ADC_COUNT 4U /*!< ADC 갯수 */
#define ADC_SAMPLE_1S_COUNT 100U /*!< Timer8(1초) 이벤트 동안 ADC 샘플 갯수 */
#define ADC_SAMPLE_COUNT (ADC_SAMPLE_1S_COUNT * ADC_COUNT) /*!< 샘플링 갯수 */

void AIO_Init(void);
float AI_Read(int8_t No);
void ADCStart(void);

#endif /* AIO_H__ */
