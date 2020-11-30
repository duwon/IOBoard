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

extern uint8_t spi1RxBuffer[10];

void AIO_Init(void);
float AI_Read(int8_t No);
void AIN_TIM_PeriodElapsedCallback(void);

void LMP90080_Test(void);
void LMP90080_GPIO_Write(uint8_t PinNum, uint8_t PinState);


void LMP90080_WriteReg(uint8_t regNum, uint8_t regData);
uint8_t LMP90080_ReadReg(uint8_t regNum);
uint8_t LMP90080_ReadRegReadAddress(uint8_t regNum);
uint16_t LMP90080_ReadReg2Byte(uint8_t regNum);
void LMP90080_ReadReg_IT(uint8_t regNum);
float LMP90080_ReadRTD(void);



void SY7T609_Test(void);
uint32_t SY7T609_ReadReg(uint8_t regNum);
uint32_t SY7T609_ReadRegIndirect(uint8_t *reg);

#endif /* AIO_H__ */
