#ifndef RTD_H__
#define RTD_H__ 1

#include "main.h"

extern uint8_t spi1RxBuffer[10];

int8_t RTD_Read(void);

void LMP90080_Test(void);
void LMP90080_GPIO_Write(uint8_t PinNum, uint8_t PinState);

void LMP90080_WriteReg(uint8_t regNum, uint8_t regData);
uint8_t LMP90080_ReadReg(uint8_t regNum);
uint8_t LMP90080_ReadRegReadAddress(uint8_t regNum);
uint16_t LMP90080_ReadReg2Byte(uint8_t regNum);
void LMP90080_ReadReg_IT(uint8_t regNum);
float LMP90080_ReadRTD(void);

#endif /* RTD_H__ */
