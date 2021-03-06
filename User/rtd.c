/**
  ******************************************************************************
  * @file    rtd.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   RTD 온도 센서
  * @details
  */

#include <string.h>
#include <stdio.h>
#include "rtd.h"
#include "gpio.h"
#include "spi.h"
#include "led.h"

/** @defgroup RTD RTD 제어 함수
  * @brief LM90080 제어 및 RTD 센싱
  * @{
  */

/** @defgroup RTD_Variable RTD 변수
  * @{
  */

static int16_t rtdValue[RTD_SENSING_AVERAGE_CNT]; /*!< RTD센싱 값 저장 버퍼 */
static int32_t rtdSum = 0;                        /*!< RTD 합계 값 */

uint8_t spi1RxBuffer[10];             /*!< AIN SPI - LMP90080 수신 버퍼 */
static uint8_t LMP9000_GPIO_REG = 0U; /*!< LM90080 GPIO 상태 값 */

static void LMP90080_Init(void);
static void LMP90080_WriteReg(uint8_t regNum, uint8_t regData);
static uint16_t LMP90080_ReadReg2Byte(uint8_t regNum);
static float LMP90080_ReadRTD(void);

/**
  * @}
  */

/**
 * @brief LMP90080 초기화
 * 
 */
static void LMP90080_Init(void)
{
  LMP9000_GPIO_REG = 0x00U;
  LMP90080_WriteReg(0x00U, 0xC3U);            /* RESET. 0xC3: Register and conversion reset */
  LMP90080_WriteReg(0x0BU, 0x01U);            /* 1: Restart conversion. */
  LMP90080_WriteReg(0x0EU, 0x3FU);            /* [6:0] Set GPIO Output */
  LMP90080_WriteReg(0x0FU, LMP9000_GPIO_REG); /* [6:0] Set GPIO Low */
  LMP90080_WriteReg(0x10, 0x02U);             /* Background Calibration Control.  BgcalMode2: Offset Correction / Gain Correction */
  LMP90080_WriteReg(0x11, 0x03U);             /* SPI DATA READY BAR CONTROL (ADDRESS 0x11) */
  LMP90080_WriteReg(0x12, 0x0AU);             /* [5] External clock detection. 1: "External-Clock Detection" is bypassed
	                                                 [4] Clock Select. 0: Internal, 1: Selects external clock
	                                                 [3:0] RTC Current Select, 1mA */
  LMP90080_WriteReg(0x17U, 0x00U);
  LMP90080_WriteReg(0x1FU, (0 << 6) + (0 << 3) + 0); /* [7:6] CH_SCAN_SEL [5:3] LAST CH [2:0] FIRST CH */
  LMP90080_WriteReg(0x20U, (1 << 6) + (0 << 3) + 1); /* CH0 configuration. [6] Select VREFP2 [5:3] AIN0 - Positive [2:0] AIN1 - Negative */
  LMP90080_WriteReg(0x21U, (7 << 4) + (3 << 1));     /* CH1 SPS 7, Gain 0:0, 1:2, 2:4, 3:8, 4:16 */
}

/**
 * @brief LMP90080의 RTD ADC 값 읽어 계싼
 * 
 * @return float: 온도 값 
 */
static float LMP90080_ReadRTD(void)
{
  float temperature;

  temperature = (float)LMP90080_ReadReg2Byte(0x1A);                                       /* ADC 값 읽기 */
  temperature = temperature * 4000 / 65535 / 8;                                           /* 저항 값 계산 */
  temperature = ((temperature * temperature) * 0.0011) + (temperature * 2.3368) - 244.58; /* 온도 계산 */

  return temperature;
}

/**
 * @brief LMP90080의 레지스터 1byte 쓰기
 * 
 * @param regNum 
 * @param regData 
 */
static void LMP90080_WriteReg(uint8_t regNum, uint8_t regData)
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

/**
 * @brief LMP90080의 레지스터 2byte 읽기
 * 
 * @param regNum: 레지스터 순번 
 * @return uint16_t: 레지스터 값
 */
static uint16_t LMP90080_ReadReg2Byte(uint8_t regNum)
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

/**
 * @brief LMP90080의 GPIO 제어
 * 
 * @param PinNum: 0~2 
 * @param PinState: 0 or 1
 */
void LMP90080_GPIO_Write(uint8_t PinNum, uint8_t PinState)
{
  LMP9000_GPIO_REG &= ~(1U << PinNum);
  LMP9000_GPIO_REG |= (PinState << PinNum);
  LMP90080_WriteReg(0x0FU, LMP9000_GPIO_REG);
}

/**
 * @brief RTD 센서 초기화
 * 
 */
void RTD_Init(void)
{
  LMP90080_Init();
}

/**
 * @brief 3선식 RTD 온도센서의 현재 온도값을 읽는다.
 * 
 * @return int16_t @ 온도 값 * 100
 */
int16_t RTD_Read(void)
{
  int16_t temperature = 0;

  temperature = rtdSum / (RTD_SENSING_AVERAGE_CNT - 1);
  rtdSum = 0;
  memset((void *)rtdValue, 0, sizeof(rtdValue));
  LED_Off(LD_RTD);
  return temperature;
}

/**
 * @brief RTD 센싱
 * 
 */
void RTDStart(void)
{
  static uint8_t Cnt = 0;
  int16_t val;

  /* RTD값 합계 구하기 */
  val = (int16_t)(LMP90080_ReadRTD() * 100);

  if ((val < -20000) || (val > 30000)) /* -200 ~ 300 값만 정상 */
  {
    return;
  }

  rtdValue[Cnt++] = val;
  rtdSum += val;
  if (Cnt >= RTD_SENSING_AVERAGE_CNT)
  {
    Cnt = 0;
    LED_On(LD_RTD);
  }
  rtdSum -= rtdValue[Cnt];
}

/**
  * @}
  */
