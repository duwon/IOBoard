#ifndef LED_H__
#define LED_H__ 1

#include "main.h"
#include "gpio.h"

typedef enum 
{
  LD_DI1 = 0,
  LD_DI2,
  LD_DI3,
  LD_DI4,
  LD_AI1,
  LD_AI2,
  LD_RS485RDY,
  LD_RS232RDY,
  LD_DO1,
  LD_DO2,
  LD_AO,
  LD_CC
} Led_TypeDef;

void LED_On(Led_TypeDef Led);
void LED_Off(Led_TypeDef Led);
void LED_Toggle(Led_TypeDef Led);
void LED_Init(void);

#endif /* LED_H__ */
