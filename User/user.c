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
#include "dio.h"

#pragma pack(push, 1) /* 1바이트 크기로 정렬  */
typedef struct
{
  uint8_t Do[2];      /* 디폴트 설정값  0=Off */
  uint8_t Rtd_Cycle;  /* 측정주기 sec	  초기값 10 */
  uint8_t Ai_Cycle;   /* 측정주기 sec   초기값 1 */
  uint8_t Di_Cycle;   /* 측정주기 sec   초기값 1 */
  uint8_t Dps_Cycle;  /* 측정주기 sec   초기값 1 */
  uint8_t Ps_Cycle;   /* 측정주기 sec   초기값 1 */
  uint8_t Pm_Mode;    /* 전산적력 측정 방식 0=3상, 1=단상 */
  uint8_t Pm_Cycle;   /* 측정주기 sec   초기값 60 */
  uint16_t Pm_Volt;   /* 파워메터 기준진압                 초기값 220 */
  uint8_t Pm_Current; /* 파워메터 기전전류  100 -> 10.0 A  초기값 50 */
  uint8_t Pm_Freq;    /* 파워메터 기준주파수               초기값 60 */
} Io_Config_TypeDef;
#pragma pack(pop)

typedef struct
{
  //-------------------------------- PowerMeter
  float Volt;          /*!< 전압 */
  float Current;       /*!< 전류 */
  float Cos;           /*!< 역률 */
  float Active;        /*!< 유효전력 */
  float Reactive;      /*!< 무효전력 */
  float Apparent;      /*!< 피상전력 */
  float Active_Energy; /*!< 유효전력량 */
  //-------------------------------- Odd I/O 상태값
  uint16_t Rtd;   /*!< MSB=정수, LSB=소수점 */
  uint16_t Dps;   /*!< MSB=정수, LSB=소수점  차압센서 */
  uint16_t Ps;    /*!< MSB=정수, LSB=소수점  압력센서 */
  uint16_t Ai[2]; /*!< MSB=정수, LSB=소수점 */
  uint8_t Di[4];  /*!< 0=Off, 1=On */
  uint8_t Do[2];  /*!< 0=Off, 1=On */
} Io_Status_TyeDef;

static Io_Config_TypeDef stIOConfig; /*!< IO Board 설정값 */
static Io_Status_TyeDef stIOStatus;  /*!< IO Board 상태정보 */

static uint32_t Time_1sec_Count, Raspberry_Timer, Restart_Timer;
static uint8_t Reset_sw = 0; // Reset switch

static void user1msLoop(void);
static void Check_Todo(void);

static void setDefaultConfig(void);
static void RasPI_Proc(void);
static void Rs232_Proc(void);
static void Di_Proc(void);

/**
 * @brief 사용자 시작 함수 - 시작시 1회 수행
 * 
 */
void userStart(void)
{
#ifdef DEBUG
  printf("\r\nstart.. %s %s\r\n", __DATE__, __TIME__);
#endif
  LED_Init();
  RTC_Load();   /* RTC IC와 내부 RTC에 동기화 */
  Uart_Init();  /* UART 통신 시작 */
  Timer_Init(); /* 타이머 시작 */

  memcpy(&stIOConfig, (unsigned char *)FLASH_SYSTEM_CONFIG_ADDRESS, sizeof(stIOConfig)); /*  IO보드 설정값 불러 오기 */
  if ((stIOConfig.Pm_Freq == 0) || stIOConfig.Pm_Freq == 255)
  {
    setDefaultConfig();
  }
  //initMessage(&uart4Message, TTL_Proc);
}

/**
 * @brief 사용자 Loop 함수
 * 
 */
void userLoop(void)
{
  procMesssage(&uart4Message, &uart4Buffer); /* 라즈베리파이(UART4) 메시지 처리 */
  procMesssage(&uart1Message, &uart1Buffer); /* RS232(UART1) 메시지 처리 */

  if (flag_100uSecTimerOn == true) /* 100us 마다 ADC */
  {
    flag_100uSecTimerOn = false;
    AIN_TIM_PeriodElapsedCallback();
  }

  if (flag_1mSecTimerOn == true) /* 1ms 마다 1ms Loop 함수 호출 */
  {
    flag_1mSecTimerOn = false;
    user1msLoop();
  }
}

/**
 * @brief 1ms 간격 수행 사용자 Loop 함수
 * 
 */
static void user1msLoop(void)
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
    Rs232_Proc();
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
    Di_Proc();
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

/**
 * @brief 1초에 한 번 수행
 * 
 */
static void Check_Todo(void)
{
  static int8_t loop = 0;

  //Set_RDY(Time_1sec_Count % 2);			// 1초 간격 브링크

  switch (loop++)
  {
  case 0: //------------------------------ Reset SW
    switch (Reset_sw)
    {
    case 1:
      Reset_sw = 0;
      break;
    case 2: // 3 초 이상
      Reset_sw = 0;
      //Factory_Cfg ();
      break;
    }
    break;
    
  case 1: //------------------------------ Restart
    if (Restart_Timer)
    {
      //if (Restart_Timer < Time_1sec_Count) Restart ();
    }
    break;

  case 2: //------------------------------ 라즈베리와의 통신 시간
    if (Raspberry_Timer < Time_1sec_Count)
    {

      ; // 라즈베리 리셋 (전원 2초간 OFF 후 ON)

      Raspberry_Timer = Time_1sec_Count + 3600; // 1시간 설정
    }
  }
}

/**
 * @brief Set the Default Config object
 *
 */
static void setDefaultConfig(void)
{
  stIOConfig.Do[0] = 0;       /*!< 디폴트 설정값  0=Off */
  stIOConfig.Do[1] = 0;       /*!< 디폴트 설정값  0=Off */
  stIOConfig.Rtd_Cycle = 10;  /*!< 측정주기 sec	  초기값 10 */
  stIOConfig.Ai_Cycle = 1;    /*!< 측정주기 sec   초기값 1 */
  stIOConfig.Di_Cycle = 1;    /*!< 측정주기 sec   초기값 1 */
  stIOConfig.Dps_Cycle = 1;   /*!< 측정주기 sec   초기값 1 */
  stIOConfig.Ps_Cycle = 1;    /*!< 측정주기 sec   초기값 1 */
  stIOConfig.Pm_Mode = 1;     /*!< 전산적력 측정 방식 0=3상, 1=단상 */
  stIOConfig.Pm_Cycle = 60;   /*!< 측정주기 sec   초기값 60 */
  stIOConfig.Pm_Volt = 220;   /*!< 파워메터 기준진압                 초기값 220 */
  stIOConfig.Pm_Current = 50; /*!< 파워메터 기전전류  100 -> 10.0 A  초기값 50 */
  stIOConfig.Pm_Freq = 60;    /*!< 파워메터 기준주파수               초기값 60 */
}

/**
 * @brief 라즈베리파이 통신 메시지 처리
 *
 */
static void RasPI_Proc(void)
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
      RTC_Get(txData); /* txData[5:0] */
      txData[6] = VERSION_MAJOR;
      txData[7] = VERSION_MINOR;
      sendMessageToRasPi(MSGCMD_RESPONSE_TIME, txData, 6U + 2U);
      break;
    case MSGCMD_UPDATE_CONFIG:
      //printf("MSGCMD_UPDATE_CONFIG\r\n");
      if (memcmp(&stIOConfig, uart4Message.data, sizeof(stIOConfig)) != 0)
      {
        memcpy(&stIOConfig, uart4Message.data, sizeof(stIOConfig));
      }
      System_Config_Write((uint32_t *)&stIOConfig, sizeof(stIOConfig));
      sendMessageToRasPi(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
      break;
    case MSGCMD_REQUEST_CONFIG:
      //printf("MSGCMD_REQUEST_CONFIG\r\n");
      sendMessageToRasPi(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
      break;
    case MSGCMD_UPDATE_TIME:
      //printf("MSGCMD_UPDATE_TIME\r\n");
      RTC_Set(uart4Message.data);
      RTC_Get(txData); /* txData[5:0] */
      txData[6] = VERSION_MAJOR;
      txData[7] = VERSION_MINOR;
      sendMessageToRasPi(MSGCMD_RESPONSE_TIME, txData, 6U + 2U);
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
      //printf("MSGCMD_SET_DO\r\n");
      DO_Write(0, uart4Message.data[0]);
      DO_Write(1, uart4Message.data[1]);
      stIOStatus.Do[0] = uart4Message.data[0];
      stIOStatus.Do[1] = uart4Message.data[1];
      break;
    case 0x99: /* 리셋 */
      NVIC_SystemReset();
      break;
    default: /* 메시지 없음 */
      printf("MSG ERROR\r\n");
      break;
    }
    uart4Message.nextStage = START;
  }

  if ((flag_sendStatusTimerOn == true) && (flag_receiveFWImage == false))
  {
    flag_sendStatusTimerOn = false;
    sendMessageToRasPi(MSGCMD_INFO_IOVALUE, (uint8_t *)&stIOStatus, sizeof(stIOStatus));
  }
}

static void Rs232_Proc(void)
{
	  if (uart1Message.nextStage == PARSING)
	  {
	    uint8_t txData[MESSAGE_MAX_SIZE] = {
	        0,
	    };

	    switch (uart1Message.msgID)
	    {
	    case MSGCMD_REQUEST_TIME:
	      //printf("MSGCMD_REQUEST_TIME\r\n");
	      RTC_Get(txData); /* txData[5:0] */
	      txData[6] = VERSION_MAJOR;
	      txData[7] = VERSION_MINOR;
	      sendMessageToRS232(MSGCMD_RESPONSE_TIME, txData, 6U + 2U);
	      break;
	    case MSGCMD_UPDATE_CONFIG:
	      //printf("MSGCMD_UPDATE_CONFIG\r\n");
	      if (memcmp(&stIOConfig, uart1Message.data, sizeof(stIOConfig)) != 0)
	      {
	        memcpy(&stIOConfig, uart1Message.data, sizeof(stIOConfig));
	      }
	      System_Config_Write((uint32_t *)&stIOConfig, sizeof(stIOConfig));
	      sendMessageToRS232(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
	      break;
	    case MSGCMD_REQUEST_CONFIG:
	      //printf("MSGCMD_REQUEST_CONFIG\r\n");
	    	sendMessageToRS232(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
	      break;
	    case MSGCMD_UPDATE_TIME:
	      //printf("MSGCMD_UPDATE_TIME\r\n");
	      RTC_Set(uart1Message.data);
	      RTC_Get(txData); /* txData[5:0] */
	      txData[6] = VERSION_MAJOR;
	      txData[7] = VERSION_MINOR;
	      sendMessageToRS232(MSGCMD_RESPONSE_TIME, txData, 6U + 2U);
	      break;
	    case MSGCMD_UPDATE_FW:
	      //printf("MSGCMD_UPDATE_FW\r\n");
	      procFirmwareUpdate(uart1Message.data);
	      break;
	    case MSGCMD_UPDATE_WATEMETER_CAL:
	      printf("MSGCMD_UPDATE_WATEMETER_CAL\r\n");
	      break;
	    case MSGCMD_REQUEST_WATMETER_VAL:
	      printf("MSGCMD_REQUEST_WATMETER_VAL\r\n");
	      break;
	    case MSGCMD_SET_DO:
	      //printf("MSGCMD_SET_DO\r\n");
	      DO_Write(0, uart1Message.data[0]);
	      DO_Write(1, uart1Message.data[1]);
	      stIOStatus.Do[0] = uart1Message.data[0];
	      stIOStatus.Do[1] = uart1Message.data[1];
	      break;
	    case 0x99: /* 리셋 */
	      NVIC_SystemReset();
	      break;
	    default: /* 메시지 없음 */
	      printf("MSG ERROR\r\n");
	      break;
	    }
	    uart1Message.nextStage = START;
	  }
}

/**
 * @brief 디지털 입력 처리
 *
 */
static void Di_Proc(void)
{
  static uint32_t NT;
  uint32_t CT;

  CT = HAL_GetTick();
  if (CT > NT)
  {
    DI_Read(stIOStatus.Di);
    NT = CT + ((uint32_t)stIOConfig.Di_Cycle << 10U);
  }
}
