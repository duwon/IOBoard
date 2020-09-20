#ifndef DIO_H__
#define DIO_H__ 1

#include "main.h"
#include "gpio.h"

void Dio_Init(void);
void DO_Write(int8_t No, int8_t Value);
uint8_t DI_Read(void);

#endif /* DIO_H__ */
