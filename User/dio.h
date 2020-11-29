#ifndef DIO_H__
#define DIO_H__ 1

#include "main.h"
#include "gpio.h"

#define RASPI_OFF HAL_GPIO_WritePin(EN_RASPI_GPIO_Port, EN_RASPI_Pin, GPIO_PIN_RESET)
#define RASPI_ON HAL_GPIO_WritePin(EN_RASPI_GPIO_Port, EN_RASPI_Pin, GPIO_PIN_SET)

void Dio_Init(void);
void DO_Write(int8_t No, int8_t Value);
void DI_Read(uint8_t *returnValue);

#endif /* DIO_H__ */
