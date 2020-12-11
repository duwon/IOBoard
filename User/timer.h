#ifndef TIMER_H__
#define TIMER_H__ 1

#include "main.h"

#define SEND_STATUS_INTERVAL 1000 /*!< 상태 전송 주기. 단위 ms */

extern bool flag_1SecTimerOn;
extern bool flag_1mSecTimerOn;
extern bool flag_10mSecTimerOn;
extern bool flag_sendStatusTimerOn;

void Timer_Init(void);
void RTC_Load(void);
void RTC_Get(uint8_t *pTime);
void RTC_Set(uint8_t *pTime);

#endif /* TIMER_H__ */
