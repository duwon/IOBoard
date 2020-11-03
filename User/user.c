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
#include "iwdg.h"
#include "tim.h"

#include "user.h"
#include "timer.h"
#include "util.h"
#include "uart.h"
#include "aio.h"
#include "flash.h"

uint32_t Time_1sec_Count, Raspberry_Timer, Restart_Timer;
uint8_t		Reset_sw = 0;						// Reset switch 

void user1msLoop(void);
void Check_Todo (void);

void setDefaultConfig(void);
void RasPI_Proc(void);

/**
 * @brief 사용자 시작 함수 - 시작시 1회 수행
 * 
 */
void userStart(void)
{
  RTC_Load(); /* RTC IC와 내부 RTC에 동기화 */
  Uart_Init();
  Timer_Init();

  //initMessage(&uart4Message, TTL_Proc);
}

/**
 * @brief 사용자 Loop 함수
 * 
 */
void userLoop(void)
{
  procMesssage(&uart4Message, &uart4Buffer);
  if(flag_100uSecTimerOn == true)
  {
    flag_100uSecTimerOn = false;
    AIN_TIM_PeriodElapsedCallback();
  }
  if(flag_1mSecTimerOn == true)
  {
    flag_1mSecTimerOn = false;
    user1msLoop();
  }
}

/**
 * @brief 1ms 간격 수행 사용자 Loop 함수
 * 
 */
void user1msLoop(void)
{
  static uint8_t looping = 0;


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
    RasPI_Proc();
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

void setDefaultConfig(void)
{
  stIOConfig.Do[0] = 0;       // 디폴트 설정값  0=Off
  stIOConfig.Do[1] = 0;       // 디폴트 설정값  0=Off
  stIOConfig.Rtd_Cycle = 10;  // 측정주기 sec	  초기값 10
  stIOConfig.Ai_Cycle = 1;    // 측정주기 sec   초기값 1
  stIOConfig.Di_Cycle = 1;    // 측정주기 sec   초기값 1
  stIOConfig.Dps_Cycle = 1;   // 측정주기 sec   초기값 1
  stIOConfig.Ps_Cycle = 1;    // 측정주기 sec   초기값 1
  stIOConfig.Pm_Mode = 1;     // 전산적력 측정 방식 0=3상, 1=단상
  stIOConfig.Pm_Cycle = 60;   // 측정주기 sec   초기값 60
  stIOConfig.Pm_Volt = 220;   // 파워메터 기준진압                 초기값 220
  stIOConfig.Pm_Current = 50; // 파워메터 기전전류  100 -> 10.0 A  초기값 50
  stIOConfig.Pm_Freq = 60;    // 파워메터 기준주파수               초기값 60
}

void RasPI_Proc(void)
{
  static bool flag_receiveFWImage = false;

  if (uart4Message.nextStage == PARSING)
  {
    uint8_t txData[MESSAGE_MAX_SIZE] = {
        0,
    };

    switch (uart4Message.msgID)
    {
    case MSGCMD_REQUEST_TIME:
      //printf("MSGCMD_REQUEST_TIME\r\n");
      RTC_Get(txData);
      sendMessageToRasPi(MSGCMD_RESPONSE_TIME, txData, 6);
      break;
    case MSGCMD_UPDATE_CONFIG:
      printf("MSGCMD_UPDATE_CONFIG\r\n");
      break;
    case MSGCMD_REQUEST_CONFIG:
      //printf("MSGCMD_REQUEST_CONFIG\r\n");
      setDefaultConfig();
      sendMessageToRasPi(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
      break;
    case MSGCMD_UPDATE_TIME:
      RTC_Set(uart4Message.data);
      printf("MSGCMD_UPDATE_TIME\r\n");
      break;
    case MSGCMD_UPDATE_FW:
      //printf("MSGCMD_UPDATE_FW\r\n");
      flag_receiveFWImage = true;
      procFirmwareUpdate(uart4Message.data);
      break;
    case MSGCMD_UPDATE_WATEMETER_CAL:
      printf("MSGCMD_UPDATE_WATEMETER_CAL\r\n");
      break;
    case MSGCMD_REQUEST_WATMETER_VAL:
      printf("MSGCMD_REQUEST_WATMETER_VAL\r\n");
      break;
    case MSGCMD_SET_DO:
      printf("MSGCMD_SET_DO\r\n");
      break;
    case 0x99:
      NVIC_SystemReset();
      break;
    default:
      printf("MSG ERROR\r\n");
      break;
    }
    uart4Message.nextStage = START;
  }

  if((flag_sendStatusTimerOn == true) && (flag_receiveFWImage == false))
  {
	  flag_sendStatusTimerOn = false;
	  sendMessageToRasPi(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOStatus, sizeof(stIOStatus));
  }
}
