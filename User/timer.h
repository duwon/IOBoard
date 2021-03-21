#ifndef TIMER_H__
#define TIMER_H__ 1

#include "main.h"

#define SEND_STATUS_INTERVAL 50 				/*!< 상태 전송 주기. 단위 100ms */

extern bool flag_1SecTimerOn;
extern bool flag_1mSecTimerOn;
extern bool flag_10mSecTimerOn;
extern bool flag_sendStatusTimerOn;
extern bool flag_saveEveragePower;
extern uint32_t Timer_10ms_Cnt;
extern uint32_t Timer_100ms_Cnt;

void Timer_Init(void);
void RTC_Load(void);
void RTC_Get(uint8_t *pTime);
void RTC_Set(uint8_t *pTime);

uint64_t RTC_LoadValue(void);
void RTC_SaveValue(uint64_t saveValue);

#endif /* TIMER_H__ */
