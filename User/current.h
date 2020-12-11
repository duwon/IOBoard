#ifndef CURRENT_H__
#define CURRENT_H__ 1

#include "main.h"

void SY7T609_Test(void);
uint32_t SY7T609_ReadReg(uint8_t regNum);
void SY7T609_WriteReg(uint8_t regNum, uint32_t regData);

float Current_Read(int8_t No);


#endif /* CURRENT_H__ */
