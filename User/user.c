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
#include "led.h"
#include "rtd.h"
#include "emp.h"
#include "ps.h"

#pragma pack(push, 1) /* 1바이트 크기로 정렬  */
typedef struct
{
	uint8_t Do[2];      /*!< 디폴트 설정값  0=Off */
	uint8_t Rtd_Cycle;  /*!< 측정주기 sec	  초기값 60 */
	uint8_t Ai_Cycle;   /*!< 측정주기 sec   초기값 1 */
	uint8_t Di_Cycle;   /*!< 측정주기 sec   초기값 1 */
	uint8_t Dp_Cycle;   /*!< 측정주기 sec   초기값 5 압력센서 */
	uint8_t Ps_Cycle;   /*!< 측정주기 sec   초기값 5 차압센서 */
	uint8_t Pm_Cycle;   /*!< 측정주기 sec   초기값 60 */
	uint16_t Pm_Volt;   /*!< 파워메터 기준진압                 초기값 220 */
	uint8_t Pm_Current; /*!< 파워메터 기준전류  100 -> 10.0 A  초기값 50 */
	uint8_t Pm_Hz;      /*!< 파워메터 기준주파수               초기값 60 */
	uint8_t Ratio;      /*!< 외부 CT 배율    6배=(30/5) ~ 140배(700/5) */
	uint8_t Volt;       /*!< 입력전압	   0=220, 1=380, 2=460 */
	uint8_t Phase;      /*!< 상종류	   0=단상, 1=3상 */
	uint8_t Pf;         /*!< Power Factor   기본 값 65 -> 0.65 */
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
	uint32_t Active_Energy; /*!< 유효전력량 */
	//-------------------------------- Odd I/O 상태값
	int16_t Rtd;         /*!< MSB=정수, LSB=소수점  온도*/
	uint16_t Dp;         /*!< MSB=정수, LSB=소수점  차압센서 */
	uint16_t Ps;         /*!< MSB=정수, LSB=소수점  압력센서 */
	uint16_t Ai[2];      /*!< MSB=정수, LSB=소수점 */
	uint8_t Di[4];       /*!< 0=Off, 1=On */
	uint8_t Do[2];       /*!< 0=Off, 1=On */
} Io_Status_TyeDef;


struct Serial_Buffer
{
	uint8_t  Buff[256];
	uint8_t  Len;
	uint32_t Timer;
};

struct Serial_Buffer  Rs485;
struct Serial_Buffer  Rs232;
struct Serial_Buffer  RsRas;

static Io_Config_TypeDef stIOConfig; /*!< IO Board 설정값 */
static Io_Status_TyeDef stIOStatus;  /*!< IO Board 상태정보 */

static uint16_t Raspberry_Timer = 0; /*!< 라즈베리파이 통신 불량시 카운트. 단위 : 초 */
static bool flag_receiveFWImage = false; /*!< 펌웨어 업데이트 중 상태 플래그 */

static void user1msLoop(void);
static void Check_Todo(void);
static void IoPorts (void);
static void setDefaultConfig(void);
static void Proc_RasPI(int len);
static void Proc_Rs232(int len);
static void Proc_Emp(void);
static void Detect_ResetSW(void);
int16_t Uart_Read (int8_t no, struct Serial_Buffer *Rs);
//==================================================================================
void userStart(void)
{
#ifdef DEBUG
	printf("\r\nstart.. %s %s\r\n", __DATE__, __TIME__);
#endif
	AIO_Init();   /* 내부 ADC 초기화 */
	RTC_Load();   /* RTC IC와 내부 RTC에 동기화 */
	RTD_Init();   /* RTD 센서 초기화 */
	Uart_Init();  /* UART 통신 시작 */
	LED_Init();   /* LED 초기화 */
	Timer_Init(); /* 타이머 시작 */
	EMP_Init();   /* EMP(전력측정) 초기화 */
	RASPI_ON;     /* 라즈베리파이 전원 출력 */

	memcpy(&stIOConfig, (unsigned char *)FLASH_SYSTEM_CONFIG_ADDRESS, sizeof(stIOConfig)); /*  IO보드 설정값 불러 오기 */
	if ((stIOConfig.Pm_Hz == 0) || stIOConfig.Pm_Hz == 255)                                /* IO보드 설정값이 없으면 기본 값으로 설정 */
	{
		setDefaultConfig();
	}
	EMP_SetDefaultValue(stIOConfig.Ratio, stIOConfig.Volt, stIOConfig.Phase, stIOConfig.Pf); /* CT 배율, 입력전압, 상종류 설정 */


	//==================================== DO 포트 초기화
	DO_Write(0, stIOConfig.Do[0]);
	stIOStatus.Do[0] = stIOConfig.Do[0];
	DO_Write(1, stIOConfig.Do[1]);
	stIOStatus.Do[1] = stIOConfig.Do[1];
}
//==================================================================================
void userLoop(void)
{
int16_t len;

	len = Uart_Read (RS_Ras, (struct Serial_Buffer *)&RsRas);
	if (len) Proc_RasPI (len);

	if (flag_saveEveragePower) /* 16ms마다 전력 센싱 */
		{
		EMP_SaveEveragePower();
		flag_saveEveragePower = false;
		}

	if (flag_receiveFWImage==false && flag_1mSecTimerOn) /* 1ms 마다 1msLoop 함수 호출 */
		{
		user1msLoop();    				/* 사용자 1ms Loop함수 호출 */
		flag_1mSecTimerOn = false;
		}

	if (flag_10mSecTimerOn) 			/* 10ms 주기적 센싱 실행 */
		{
		Detect_ResetSW(); 				/* RESET 스위치 입력 확인 */
		IoPorts ();
		ADCStart(); /* ADC 수행 - AIN, DS */
		RTDStart(); /* RTD 수행 */
		PSStart();  /* DP 수행  */
		flag_10mSecTimerOn = false;
		}

	if (flag_1SecTimerOn) /* 1초 간격 처리 */
		{
		Check_Todo();
		flag_1SecTimerOn = false;
		}
}
//==================================================================================
static void IoPorts (void)
{
static uint8_t looping = 0;
static uint32_t t100ms = 10;

	if (t100ms == Timer_100ms_Cnt) return;
	t100ms = Timer_100ms_Cnt;

	switch (looping++)
	{
	case 0: //---------------------------------------------< 100 ms 간격 처리
		break;
	case 1: //----------------------------------------------< LoRa
		//LoRa_Proc();
		break;
	case 2: //----------------------------------------------< DI
		DI_Read(stIOStatus.Di);
		break;
	case 3: //----------------------------------------------< DO
		DO_Write(0, stIOStatus.Do[0]);
		DO_Write(1, stIOStatus.Do[1]);
		break;
	case 4: //----------------------------------------------< AI
		stIOStatus.Ai[0] = AI_Read(0);
		stIOStatus.Ai[1] = AI_Read(1);
		if(stIOStatus.Ai[0] > 3000) LED_On(LD_AIN1); else LED_Off(LD_AIN1);
		if(stIOStatus.Ai[1] > 3000) LED_On(LD_AIN2); else LED_Off(LD_AIN2);
		break;
	case 5: //----------------------------------------------< RTD 센서
		stIOStatus.Rtd = RTD_Read(); 							/* 소수점 이하 추가 */
		break;
	case 6: //---------------------------------------------< 전력센서
		Proc_Emp();
		break;
	case 7: //---------------------------------------------< 차압센서
		stIOStatus.Dp = DP_Read(); /* 소수점 이하 추가 */
		break;
	case 8: //---------------------------------------------< 압력센서
		stIOStatus.Ps = PS_Read();
		break;
	default:
		looping = 0;
		break;
	}
}
//==================================================================================
static void user1msLoop(void)
{
uint16_t len;

	len = Uart_Read (RS_232, (struct Serial_Buffer *)&Rs232);
	if (len) Proc_Rs232 (len);

	len = Uart_Read (RS_485, (struct Serial_Buffer *)&Rs485);
	if (len)
		{
		Uart_Send (RS_485, "Receive = (", 11);
		Rs485.Buff [len++] = ')';
		Rs485.Buff [len++] = 0x0d;
		Rs485.Buff [len++] = 0x0a;
		Uart_Send (RS_485, Rs485.Buff, len);
		}
}
//==================================================================================
static void Check_Todo(void)
{
static uint8_t Send_1sec_Timer = 0;

	HAL_IWDG_Refresh(&hiwdg);
	LED_Toggle(LD_RDY); /* 1초 간격 LED 토글 */

	//------------------------------ send status
	if (++Send_1sec_Timer >= 27) 
		{
		Send_1sec_Timer = 22;
		if (flag_receiveFWImage == false)
			{
			sendMessageToRasPi(MSGCMD_INFO_IOVALUE, (uint8_t *)&stIOStatus, sizeof(stIOStatus));
			}
		}

	Raspberry_Timer++; 				/* 라즈베리 통신 시간 체크 */
	//------------------------------ 라즈베리와의 통신 시간
	if (Raspberry_Timer > 1805)
		{
		RASPI_ON; 						// 1802초에 라즈베리 전원 ON (전원 2초간 OFF 후 ON)
		Raspberry_Timer = 0;
		}
	
	if (Raspberry_Timer > 1800)
		{
		RASPI_OFF; 						// 1800초 동안 응답 없으면 라즈베리 전원  OFF
		}
}
//==================================================================================
static void setDefaultConfig(void)
{
	stIOConfig.Do[0] = 0;       /*!< 디폴트 설정값  0=Off */
	stIOConfig.Do[1] = 0;       /*!< 디폴트 설정값  0=Off */
	stIOConfig.Rtd_Cycle =2;    /*!< 측정주기 sec	  초기값 60 */
	stIOConfig.Ai_Cycle = 1;    /*!< 측정주기 sec   초기값 1 */
	stIOConfig.Di_Cycle = 1;    /*!< 측정주기 sec   초기값 1 */
	stIOConfig.Dp_Cycle = 2;    /*!< 측정주기 sec   초기값 5   차압센서  (0 = Disable) */
	stIOConfig.Ps_Cycle = 2;    /*!< 측정주기 sec   초기값 5   압력센서  (0 = Disable) */
	stIOConfig.Pm_Cycle = 3;    /*!< 측정주기 sec   초기값 15 */
	stIOConfig.Pm_Volt = 220;   /*!< 파워메터 기준진압                 초기값 220 */
	stIOConfig.Pm_Current = 10; /*!< 파워메터 기전전류  100 -> 10.0 A  초기값 50 */
	stIOConfig.Pm_Hz = 60;      /*!< 파워메터 기준주파수               초기값 60 */
	stIOConfig.Ratio = 1;
	stIOConfig.Volt = 0;
	stIOConfig.Phase = 0;
	stIOConfig.Pf = 100;
}
//---------------------------------------------------------------------------------------------
uint8_t Get_Cheksum (uint8_t *p, int len)
{
	uint8_t sum = 0;
	for (int i=0; i<len; i++) sum ^= *p++;
	return sum;	
}
//---------------------------------------------------------------------------------------------
static void Proc_RasPI(int len)
{
	uint8_t txData[MESSAGE_MAX_SIZE] = {	0,	};

	if (RsRas.Buff [0] != MESSAGE_STX)  return;
	if (RsRas.Buff [2] != len-5)  return;
	if (RsRas.Buff[len-1] != Get_Cheksum (RsRas.Buff, len-1)) return;

	switch (RsRas.Buff[1])
	{
	case MSGCMD_REQUEST_TIME:
		Raspberry_Timer = 0U;
		RTC_Get(txData); /* txData[5:0] */
		txData[6] = (VERSION_MAJOR * 10U) + VERSION_MINOR;
		sendMessageToRasPi(MSGCMD_RESPONSE_TIME, txData, 6U + 1U);
		break;
	case MSGCMD_UPDATE_CONFIG:
		if (memcmp(&stIOConfig, &RsRas.Buff[3], sizeof(stIOConfig)) != 0)
		{
			memcpy(&stIOConfig, &RsRas.Buff[3], sizeof(stIOConfig));
			System_Config_Write((uint32_t *)&stIOConfig, sizeof(stIOConfig)); // (config 변경 시만 저장하도록)
			EMP_SetDefaultValue(stIOConfig.Ratio, stIOConfig.Volt, stIOConfig.Phase, stIOConfig.Pf);
		}
		sendMessageToRasPi(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
		break;
	case MSGCMD_REQUEST_CONFIG:
		sendMessageToRasPi(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
		break;
	case MSGCMD_UPDATE_TIME:
		RTC_Set(&RsRas.Buff[3]);
		RTC_Get(txData); /* txData[5:0] */
		txData[6] = (VERSION_MAJOR * 10U) + VERSION_MINOR;
		sendMessageToRasPi(MSGCMD_RESPONSE_TIME, txData, 6U + 1U);
		break;
	case MSGCMD_UPDATE_FW:
		flag_receiveFWImage = true;
		procFirmwareUpdate(&RsRas.Buff[3]);
		break;
	case MSGCMD_CAL_WATEMETER:
		SY7T609_Cal((uint32_t)stIOConfig.Pm_Volt * 1000, (uint32_t)stIOConfig.Pm_Current * 100); /* mV, mA */
		break;
	case MSGCMD_CAL_WATEMETER_OFFSET:
		SY7T609_Cal_Offset();
		break;
	case MSGCMD_REQUEST_CAL_VALUE:
		EMP_GetCalValue(txData);
		sendMessageToRasPi(MSGCMD_RESPONSE_CAL_VALUE, txData, 8U);
		break;
	case MSGCMD_UPDATE_CAL_VALUE:
		EMP_UpdateCalValue(&RsRas.Buff[3]);
		break;
	case MSGCMD_RESET_WATEMTER_VAL:
		sumPowerMeter = 0;
		RTC_SaveValue(sumPowerMeter); /* RTC SRAM에 저장된 값을 초기화 */
		break;
	case MSGCMD_SET_DO:
		stIOStatus.Do[0] = RsRas.Buff[3];
		stIOStatus.Do[1] = RsRas.Buff[4];
		break;
	case 0x99: /* 리셋 */
		NVIC_SystemReset();
		break;
	default: /* 메시지 없음 */
		printf("MSG ERROR\r\n");
		break;
    }
}
//------------------------------------------------------------------------------
static void Proc_Rs232(int len)
{
	uint8_t txData[MESSAGE_MAX_SIZE] = {	0,	};
	uint32_t tmpData = 0;

	if (Rs232.Buff [0] != MESSAGE_STX)  return;
	if (Rs232.Buff [2] != len-5)  return;
	if (Rs232.Buff[len-1] != Get_Cheksum (Rs232.Buff, len-1)) return;

	switch (Rs232.Buff[1])
	{
	case MSGCMD_REQUEST_TIME:
		Raspberry_Timer = 0U;
		RTC_Get(txData); /* txData[5:0] */
		txData[6] = (VERSION_MAJOR * 10U) + VERSION_MINOR;
		sendMessageToRS232(MSGCMD_RESPONSE_TIME, txData, 6U + 1U);
		break;
	case MSGCMD_UPDATE_CONFIG:
		if (memcmp(&stIOConfig, &Rs232.Buff[3], sizeof(stIOConfig)) != 0)
		{
			memcpy(&stIOConfig, &Rs232.Buff[3], sizeof(stIOConfig));
			System_Config_Write((uint32_t *)&stIOConfig, sizeof(stIOConfig));
			EMP_SetDefaultValue(stIOConfig.Ratio, stIOConfig.Volt, stIOConfig.Phase, stIOConfig.Pf);
		}
		sendMessageToRS232(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
		break;
	case MSGCMD_REQUEST_CONFIG:
		sendMessageToRS232(MSGCMD_RESPONSE_CONFIG, (uint8_t *)&stIOConfig, sizeof(stIOConfig));
		break;
	case MSGCMD_UPDATE_TIME:
		RTC_Set(&Rs232.Buff[3]);
		RTC_Get(txData); /* txData[5:0] */
		txData[6] = (VERSION_MAJOR * 10U) + VERSION_MINOR;
		sendMessageToRS232(MSGCMD_RESPONSE_TIME, txData, 6U + 1U);
		break;
	case MSGCMD_UPDATE_FW:
		flag_receiveFWImage = true;
		procFirmwareUpdate(&Rs232.Buff[3]);
		break;
	case MSGCMD_CAL_WATEMETER:
		SY7T609_Cal((uint32_t)stIOConfig.Pm_Volt * 1000, (uint32_t)stIOConfig.Pm_Current * 100); /* mV, mA */
		break;
	case MSGCMD_REQUEST_CAL_VALUE:
		EMP_GetCalValue(txData);
		sendMessageToRS232(MSGCMD_RESPONSE_CAL_VALUE, txData, 8U);
		break;
	case MSGCMD_UPDATE_CAL_VALUE:
		EMP_UpdateCalValue(&Rs232.Buff[3]);
		break;
	case MSGCMD_RESET_WATEMTER_VAL:
		sumPowerMeter = 0;
		RTC_SaveValue(sumPowerMeter); /* RTC SRAM에 저장된 값을 초기화 */
		break;
	case MSGCMD_SET_DO:
		stIOStatus.Do[0] = Rs232.Buff[3];
		stIOStatus.Do[1] = Rs232.Buff[4];
		break;
	case 0x99: /* 리셋 */
		NVIC_SystemReset();
		break;
	case 0xD1: /* test 1 */
		//printf("TEST1 \r\n");
		//LMP90080_Test();
		//PSENOR_Read();
		//printf("RTD : %d.%d\r\n", (int)LMP90080_ReadRTD(), (int)((LMP90080_ReadRTD() - (int)LMP90080_ReadRTD())*100));//(int)(LMP90080_ReadRTD() * 100));
		printf("psTemp : %d.%d\r\n", (int)psTemperature, (int)((psTemperature - (int)psTemperature) * 100));
		break;
	case 0xD2: /* test 2 */
		//printf("TEST2 \r\n");
		printf("0x01 : %lx\r\n", SY7T609_ReadReg(0x01));
		break;
	case 0xD3: /* SPI Write Reg */
		//LMP90080_WriteReg(Rs232.Buff[3], Rs232.Buff[4]);
		break;
	case 0xD4: /* SPI Read Reg */
		//printf("%x : %x \r\n", Rs232.Buff[3], LMP90080_ReadReg2Byte(Rs232.Buff[3]));
		break;
	case 0xD5: /* Request Status Data */
		sendMessageToRS232(MSGCMD_INFO_IOVALUE, (uint8_t *)&stIOStatus, sizeof(stIOStatus));
		break;
	case 0xD6: /* SPI Read Reg - Read Address */
		//printf("%x : %x \r\n", Rs232.Buff[3], LMP90080_ReadRegReadAddress(Rs232.Buff[3]));
		break;
	case 0xE0: /* SPI Read Reg - Read Address */
		printf("%x : 0x%lx, %d\r\n", Rs232.Buff[3], SY7T609_ReadReg(Rs232.Buff[3]), SY7T609_ReadReg(Rs232.Buff[3]));
		break;
	case 0xE1: /* SPI Read Reg - Indirect Read Address */
		SY7T609_WriteReg(Rs232.Buff[3], (Rs232.Buff[4] << 16) + (Rs232.Buff[5] << 8) + Rs232.Buff[6]);
		break;
	case 0xE2:
		sendMessageToRS232(0xE2, (uint8_t *)&sensingPower, 4);
		sendMessageToRS232(0xE4, (uint8_t *)&sumPowerMeter, 8);
		break;
	case 0xE3:
		sendMessageToRS232(0xE3, (uint8_t *)&psTemperature, 4);
		break;
	default: /* 메시지 없음 */
		printf("MSG ERROR\r\n");
		break;
	}
}
//==================================================================================
static void Proc_Emp(void)
{
static uint32_t NT=10;

	if (Timer_100ms_Cnt > NT)
	{
		EMP_Read((float *)&stIOStatus);
		stIOStatus.Active_Energy = (uint32_t)(sumPowerMeter / 1000);
		NT = Timer_100ms_Cnt + ((uint32_t)stIOConfig.Pm_Cycle * 10);
	}
}
//==================================================================================
static void Detect_ResetSW(void)
{
static uint32_t cntResetTime = 0U;

	if (HAL_GPIO_ReadPin(SW_RESET_GPIO_Port, SW_RESET_Pin) == GPIO_PIN_RESET)
		{
		cntResetTime++; 					//user1msLoop 함수의
		}
	else
		{
		if (cntResetTime > 400U) 			/* 4s */
			{
			//Factory_Cfg (); 				/* 공장초기화 실행 */
			}
		else if (cntResetTime > 100U) 		/* 1s */
			{
			NVIC_SystemReset(); 			/* 소프트웨어 리셋 실행 */
			}
		cntResetTime = 0U;
		}
}
//-----------------------------------------------------------------------------------
int16_t Uart_Read (int8_t no, struct Serial_Buffer *Rs)
{
	uint8_t ch, ret;
	int16_t len = 0;

	switch (no)
	{
	case RS_232  : ret = Uart_Receive (&uart1Buffer, &ch); break;
	case RS_485  : ret = Uart_Receive (&uart2Buffer, &ch); break;
	case RS_Ras  : ret = Uart_Receive (&uart4Buffer, &ch); break;
	}

	if (ret)
	{
		Rs->Buff[Rs->Len]	 = ch;

		if (++Rs->Len >= 256)  --Rs->Len;
		if (Timer_10ms_Cnt >= 0xffff0000) Timer_10ms_Cnt = 0;   // 한바퀴 도는거 감안
		Rs->Timer = Timer_10ms_Cnt + 3;							// 30 msec 동안 안들어오면 		
	}
	else
	{
		if (Rs->Len)
		{
			if (Rs->Timer < Timer_10ms_Cnt) 
			{
				len = Rs->Len;
				Rs->Len = 0;
			}
		}
	}

	return len;
}				