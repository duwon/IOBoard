/**
  ******************************************************************************
  * @file    user.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   메인 구동 함수
  * @details
  */

/**
  * @mainpage IO Board 
    @section intro 소개
    - IO Board 
  * @section info 정보
    - IO Board  제어 API
  * @section  CREATEINFO      작성정보
    - 작성자      :   정두원
    - 작성일      :   2020-09-18
  * @section  MODIFYINFO      수정정보
    - 2020-09-18    :    프로젝트 셋팅
  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user.h"
#include "timer.h"
#include "util.h"
#include "iwdg.h"
#include "tim.h"

uint32_t Time_1sec_Count, Raspberry_Timer, Restart_Timer;
uint8_t		Reset_sw = 0;						// Reset switch 

void Check_Todo (void);

/**
 * @brief 사용자 시작 함수 - 시작시 1회 수행
 * 
 */
void userStart(void)
{
  RTC_Load(); /* RTC IC와 내부 RTC에 동기화 */
  HAL_TIM_Base_Start_IT(&htim6); /* 1ms 타이머 인터럽트 시작 */
  HAL_TIM_Base_Start_IT(&htim7); /* 0.1ms 타이머 인터럽트 시작 */
}

/**
 * @brief 사용자 Loop 함수 
 * 
 */
void userLoop(void)
{
  uint8_t looping = 0;

Dasi:

  ADN_msleep(1);

  //--------------------- 각 case 문의 함수는 대략 10msec 마다 한번씩 수행되도록  팬딩없이 바로 리턴하는 구조로 작성해야함
  switch (looping++)
  {
  case 0: //---------------------------------------------< 1초 간격 처리
    if (flag_1SecTimerOn == true)
    {
      flag_1SecTimerOn = false;
      HAL_IWDG_Refresh(&hiwdg);
      Check_Todo();
    }
    break;
  case 1: //----------------------------------------------<
    //Rs232_Proc();
    break;
  case 2: //----------------------------------------------<
    //Rs485_Proc();
    break;
  case 3: //----------------------------------------------< 디버그 포트
    //Console_Proc();
    break;
  case 4: //----------------------------------------------< 라즈베리파이 통신
    //TTL_Proc();
    break;
  case 5: //----------------------------------------------< LoRa
    //LoRa_Proc();
    break;
  case 6: //----------------------------------------------< DI
    //Di_Proc();
    break;
  case 7: //----------------------------------------------< DO
    //Do_Proc();
    break;
  case 8: //----------------------------------------------< AI
    //Ai_Proc();
    break;
  case 9: //----------------------------------------------< RTD 센서
    //Rtd_Proc();
    break;
  case 10: //---------------------------------------------< 커런트 전류
    //Ct_Proc();
    break;
  case 11: //---------------------------------------------< 차압센서
    //Dp_Proc();
    break;

  default:
    looping = 0;
    break;
  }

  goto Dasi;
}

//-----------------------------------------------------------------------------------------
void Check_Todo (void)
{
static int8_t loop=0;

	//Set_RDY(Time_1sec_Count % 2);			// 1초 간격 브링크
		
	switch (loop++)
		{
		case 0 : //------------------------------ Reset SW 
			switch (Reset_sw)				
				{
				case 1 : 							
					Reset_sw = 0;
					break;
				case 2 :						// 3 초 이상
					Reset_sw = 0;
					//Factory_Cfg ();
					break;
				}
			break;
		case 1 : //------------------------------ Restart 
			if (Restart_Timer)				
   				{
	   			//if (Restart_Timer < Time_1sec_Count) Restart ();
	   			}
			break;
			
		case 2 : //------------------------------ 라즈베리와의 통신 시간
			if (Raspberry_Timer < Time_1sec_Count)
				{
				
				;								// 라즈베리 리셋 (전원 2초간 OFF 후 ON)
				
				Raspberry_Timer = Time_1sec_Count + 3600;		// 1시간 설정
				}
		}
}
