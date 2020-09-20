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
  LD_AO
} Led_TypeDef;


#endif /* LED_H__ */
