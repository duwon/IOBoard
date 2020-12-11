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

uint8_t spi1RxBuffer[10];                /*!< AIN SPI - LMP90080 수신 버퍼 */
static bool flag_spi1RxComplete = false; /*!< AIN SPI - LMP90080 수신 완료 플래그, false로 변경해야 수신 가능 */
static uint8_t LMP9000_GPIO_REG = 0U;

static void LMP90080_Init(void);


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
  LMP90080_WriteReg(0x1FU, (0 << 6) + (2 << 3) + 0); /* [7:6] CH_SCAN_SEL [5:3] LAST CH [2:0] FIRST CH */
  LMP90080_WriteReg(0x20U, (1 << 6) + (0 << 3) + 1); /* CH0 Config. [6] Select VREFP2 [5:3] AIN0 - Psitive [2:0] AIN1 - Negative */
  LMP90080_WriteReg(0x21U, (7 << 4) + (3 << 1));     /* CH1 SPS 7, Gain 0:0, 1:2, 3:8, 4:16 */
  LMP90080_WriteReg(0x22U, (0 << 6) + (2 << 3) + 7); /* CH1 Input. AIN2 - Psitive, AIN7 - Negative */
  LMP90080_WriteReg(0x23U, 0x70);                    /* CH2 SPS, Gain 1 */
  LMP90080_WriteReg(0x24U, (0 << 6) + (3 << 3) + 7); /* CH2 Input. AIN3 - Psitive, AIN7 - Negative */
  LMP90080_WriteReg(0x25U, 0x70);                    /* CH2 SPS, Gain 1 */
}

float LMP90080_ReadRTD(void)
{
  float temperature;

  temperature = (float)LMP90080_ReadReg2Byte(0x1A);                                       /* ADC 값 읽기 */
  temperature = temperature * 4000 / 65535 / 8;                                           /* 저항 값 계산 */
  temperature = ((temperature * temperature) * 0.0011) + (temperature * 2.3368) - 244.58; /* 온도 계산 */

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

  LMP90080_WriteReg(0x1FU, (0 << 6) | (6 << 3) | ChNum); /* [7:6] CH_SCAN_SEL [5:3] LAST CH [2:0] FIRST CH */
  LMP90080_WriteReg(0x0BU, 0x01U);                       /* 1: Restart conversion. */
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
  if (adcNum == 7)
    adcNum = 0;
}

void RTD_Init(void)
{
  LMP90080_Init();
}

/**
 * @brief 3선식 RTD 온도센서의 현재 온도값을 읽는다.
 * 
 * @return int8_t @ 온도 값 : -50~150
 */
int8_t RTD_Read(void)
{
  int8_t temperature = 0;
  return temperature;
}
