#ifndef TIMER_H__
#define TIMER_H__ 1

#include "main.h"


extern bool flag_1SecTimerOn;

void RTC_Load(void);
void RTC_Set(RTC_DateTypeDef sDate, RTC_TimeTypeDef sTime, uint32_t Format);

#endif /* TIMER_H__ */
