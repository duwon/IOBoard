/**
  ******************************************************************************
  * @file    aio.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   Analog Input 제어
  * @details

  */

#include <string.h>
#include <stdio.h>
#include "aio.h"
#include "tim.h"
#include "spi.h"

uint8_t spi1RxBuffer[10];         /*!< AIN SPI - LMP90080 수신 버퍼 */
uint8_t spi3RxBuffer[10];         /*!< EMP SPI - SY7T609 수신 버퍼 */
static bool flag_spi1RxComplete = false; /*!< AIN SPI - LMP90080 수신 완료 플래그, false로 변경해야 수신 가능 */
static bool flag_spi3RxComplete = false; /*!< EMP SPI - SY7T609 수신 완료 플래그, false로 변경해야 수신 가능 */
static void LMP90080_Init(void);

uint8_t LMP9000_GPIO_REG = 0U;



/**
 * @brief Analog Input 
 * 
 * @param No : 포트 번호 : 1, 2
 * @return float : 전류 값
 */
float AI_Read(int8_t No)
{
  float analogValue = 0.0f;
  return analogValue;
}

void AIO_Init(void)
{
	LMP90080_Init();
}

static void LMP90080_Init(void)
{
	  LMP9000_GPIO_REG = 0x00U;
	  LMP90080_WriteReg(0x00U, 0xC3U);            /* RESET. 0xC3: Register and conversion reset */
	  LMP90080_WriteReg(0x0BU, 0x01U);            /* 1: Restart conversion. */
	  LMP90080_WriteReg(0x0EU, 0x3FU);            /* [6:0] Set GPIO Output */
	  LMP90080_WriteReg(0x0FU, LMP9000_GPIO_REG); /* [6:0] Set GPIO Low */
	  LMP90080_WriteReg(0x10, 0x02U);              /* Background Calibration Control.  BgcalMode2: Offset Correction / Gain Correction */
	  LMP90080_WriteReg(0x11, 0xA3U);              /* SPI DATA READY BAR CONTROL (ADDRESS 0x11) */
	  LMP90080_WriteReg(0x12, 0x0AU);    /* [5] External clock detection. 1: "External-Clock Detection" is bypassed
	                                                 [4] Clock Select. 0: Internal, 1: Selects external clock
	                                                 [3:0] RTC Current Select, 1mA */
	  LMP90080_WriteReg(0x17U, 0x00U);
	  LMP90080_WriteReg(0x1FU, (0 << 6) + (2 << 3) + 0);             /* [7:6] CH_SCAN_SEL [5:3] LAST CH [2:0] FIRST CH */
	  LMP90080_WriteReg(0x20U, (1 << 6) + (0 << 3) + 1);   /* CH0 Config. [6] Select VREFP2 [5:3] AIN0 - Psitive [2:0] AIN1 - Negative */
	  LMP90080_WriteReg(0x21U, (7 << 4) + (3 << 1));   /* CH1 SPS 7, Gain 0:0, 1:2, 3:8, 4:16 */
	  LMP90080_WriteReg(0x22U, (0 << 6) + (2 << 3) + 7);   /* CH1 Input. AIN2 - Psitive, AIN7 - Negative */
	  LMP90080_WriteReg(0x23U, 0x70);   /* CH2 SPS, Gain 1 */
	  LMP90080_WriteReg(0x24U, (0 << 6) + (3 << 3) + 7);   /* CH2 Input. AIN3 - Psitive, AIN7 - Negative */
	  LMP90080_WriteReg(0x25U, 0x70);   /* CH2 SPS, Gain 1 */
}

float LMP90080_ReadRTD(void)
{
	float temperature;
	//static int cntADCOutNotAvailable = 0;
	//LMP90080_WriteReg(0x20U, (1 << 6) + (0 << 3) + 1);   /* CH0 Config. [6] Select VREFP2 [5:3] AIN0 - Psitive [2:0] AIN1 - Negative */
	//LMP90080_WriteReg(0x21U, (7 << 4) + (3 << 1));   /* CH1 SPS 7, Gain 0:0, 1:2, 3:8, 4:16 */
	//LMP90080_WriteReg(0x0BU, 0x01U);            /* 1: Restart conversion. */
	//while(LMP90080_ReadReg(0x18) == 0xff)
	//{
	//	cntADCOutNotAvailable++;
	//	if(cntADCOutNotAvailable >= 100)
	//		return 1000;
	//}
	temperature = (float)LMP90080_ReadReg2Byte(0x1A);
	temperature = temperature * 4000 / 65535 / 8;
	temperature = ((temperature * temperature) * 0.0011) + (temperature * 2.3368) - 244.58;

	return temperature;

}

void LMP90080_WriteReg(uint8_t regNum, uint8_t regData)
{
  uint8_t spiData[4];
  spiData[0] = 0x10U;                  /* 0x10: Write Address, 0x90: Read Address */
  spiData[1] = (regNum >> 4U) & 0x0FU; /* [4:0] Upper Address */
  spiData[2] = (0U << 7U);             /* 0: Write, 1: Read */
  spiData[2] |= (0U << 5U);            /* 1Byte Write */
  spiData[2] |= (regNum & 0x0FU);      /* [4:0] Lower Address */
  spiData[3] = regData;

  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, spiData, 4U, 0xFFFF);
  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_SET);
}

uint8_t LMP90080_ReadReg(uint8_t regNum)
{
  spi1RxBuffer[0] = 0x10U;                  /* 0x10: Write Address, 0x90: Read Address */
  spi1RxBuffer[1] = (regNum >> 4U) & 0x0FU; /* [4:0] Upper Address */
  spi1RxBuffer[2] = (1U << 7U);             /* 0: Write, 1: Read */
  spi1RxBuffer[2] |= (1U << 5U);            /* 0: 1byte, 1: 2byte, 2, nByte Read */
  spi1RxBuffer[2] |= (regNum & 0x0F);       /* [4:0] Lower Address */

  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Receive(&hspi1, spi1RxBuffer, 5U, 0xFFFF);
  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_SET);

  return spi1RxBuffer[3];
}

uint16_t LMP90080_ReadReg2Byte(uint8_t regNum)
{
  spi1RxBuffer[0] = 0x10U;                  /* 0x10: Write Address, 0x90: Read Address */
  spi1RxBuffer[1] = (regNum >> 4U) & 0x0FU; /* [4:0] Upper Address */
  spi1RxBuffer[2] = (1U << 7U);             /* 0: Write, 1: Read */
  spi1RxBuffer[2] |= (1U << 5U);            /* 0: 1byte, 1: 2byte, 2, nByte Read */
  spi1RxBuffer[2] |= (regNum & 0x0F);       /* [4:0] Lower Address */

  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Receive(&hspi1, spi1RxBuffer, 5U, 0xFFFF);
  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_SET);

  return (spi1RxBuffer[3] << 8) + spi1RxBuffer[4];
}

uint8_t LMP90080_ReadRegReadAddress(uint8_t regNum)
{
  spi1RxBuffer[0] = 0x90U;                  /* 0x10: Write Address, 0x90: Read Address */
  spi1RxBuffer[1] = (regNum >> 4U) & 0x0FU; /* [4:0] Upper Address */
  spi1RxBuffer[2] = (1U << 7U);             /* 0: Write, 1: Read */
  spi1RxBuffer[2] |= (1U << 5U);            /* 0: 1byte, 1: 2byte, 2, nByte Read */
  spi1RxBuffer[2] |= (regNum & 0x0F);       /* [4:0] Lower Address */

  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Receive(&hspi1, spi1RxBuffer, 5U, 0xFFFF);
  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_SET);

  return spi1RxBuffer[3];
}

void LMP90080_ReadReg_IT(uint8_t regNum)
{
  if (flag_spi1RxComplete == false)
  {
    spi1RxBuffer[0] = 0x10U;                  /* 0x10: Write Address, 0x90: Read Address */
    spi1RxBuffer[1] = (regNum >> 4U) & 0x0FU; /* [4:0] Upper Address */
    spi1RxBuffer[2] = (1U << 7U);             /* 0: Write, 1: Read */
    spi1RxBuffer[2] |= (1U << 5U);            /* 0: 1byte, 1: 2byte, 2, nByte Read */
    spi1RxBuffer[2] |= (regNum & 0x0F);       /* [4:0] Lower Address */

    HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Receive_IT(&hspi1, spi1RxBuffer, 5U);
  }
}

void LMP90080_GPIO_Write(uint8_t PinNum, uint8_t PinState)
{
  LMP9000_GPIO_REG &= ~(1U << PinNum);
  LMP9000_GPIO_REG |= (PinState << PinNum);
  LMP90080_WriteReg(0x0FU, LMP9000_GPIO_REG);
}

void LMP90080_GPIO_Toggle(uint8_t PinNum)
{
  LMP9000_GPIO_REG ^= (1U << PinNum);
  LMP90080_WriteReg(0x0FU, LMP9000_GPIO_REG);
}

bool flag_LMP90080_Converting = false;
uint8_t adcDone = 0;
uint8_t adcCh6Config = 0;
void LMP90080_ReadADC(uint8_t ChNum)
{
  if (ChNum > 6)
  {
    return;
  }

  LMP90080_WriteReg(0x1FU, (0 << 6) | (6 << 3) | ChNum);             /* [7:6] CH_SCAN_SEL [5:3] LAST CH [2:0] FIRST CH */
  LMP90080_WriteReg(0x0BU, 0x01U);            /* 1: Restart conversion. */
  HAL_Delay(1);
  adcDone = LMP90080_ReadReg(0x18);
  LMP90080_ReadReg(0x1A); /* 16-BIT CONVERSION DATA (TWO’S COMPLEMENT) (ADDRESS 0x1A - 0x1B) */
  adcCh6Config = LMP90080_ReadReg(0x2D);
}

uint8_t adcNum = 0;
void LMP90080_Test(void)
{
  LMP90080_GPIO_Toggle(1);
  LMP90080_GPIO_Toggle(0);
  LMP90080_WriteReg(0x12U, 0xa);


  LMP90080_ReadADC(adcNum++);
  if(adcNum == 7)
	  adcNum = 0;
}

/**
 * @brief EMP Register 쓰기
 * 
 * @param regNum:
 * @param regData:
 */
void SY7T609_WriteReg(uint8_t regNum, uint16_t regData)
{
  uint8_t spiData[5] = {0x01, regNum, (uint8_t)((regData >> 8U) & 0xFFU), (uint8_t)(regData & 0xFFU), 0};
  HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit_IT(&hspi3, spiData, 5U);
}

void SY7T609_ReadReg(uint8_t regNum)
{
  if (flag_spi3RxComplete == false)
  {
    spi3RxBuffer[0] = 0x01;
    spi3RxBuffer[1] = (regNum << 2);
    HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Receive_IT(&hspi3, spi3RxBuffer, 5U); /* [0] : 레지스터, [1] : 레지스터 값 */
  }
}

void SY7T609_Test(void)
{
  SY7T609_ReadReg(0x02);
}

/**
 * @brief SPI RX 인터럽트
 * 
 * @param hspi 
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI1) /* AIN - LMP90080 */
  {
    HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_SET);
    //flag_spi1RxComplete = true;
  }
  if (hspi->Instance == SPI2) /* LORA */
  {
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
  }
  if (hspi->Instance == SPI3) /* EMP - SY7T609 */
  {
    HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
    flag_spi3RxComplete = true;
  }
}

/**
 * @brief SPI TX 인터럽트
 * 
 * @param hspi 
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI1) /* AIN - LMP90080 */
  {
    HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_SET);
  }
  if (hspi->Instance == SPI3) /* EMP - SY7T609 */
  {
    HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
  }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
}

#include "uart.h"
/**
 * @brief 100us 인터럽트. Analog input 데이터 저장
 * 
 */
void AIN_TIM_PeriodElapsedCallback(void)
{
  if (flag_spi1RxComplete == true) /* AIN - LMP90080 데이터터 수신 완료 */
  {
    flag_spi1RxComplete = false;
    /* ADC 값 저장 - 추 후 작성 */
    if(flag_LMP90080_Converting)
    {
    	flag_LMP90080_Converting = false;
    	//LMP90080_WriteReg(0x0BU, 0x01U);            /* 1: Restart conversion. */
    }
  }

  if (flag_spi3RxComplete == true) /* AIN - LMP90080 데이터터 수신 완료 */
  {
    flag_spi3RxComplete = false;
    /* ADC 값 저장 - 추 후 작성 */
    sendMessageToRS232(MSGCMD_RESPONSE_WATMETER_VAL, spi3RxBuffer, 5);
  }
}
