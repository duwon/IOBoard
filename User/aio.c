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


static uint8_t spi1RxBuffer[10]; /*!< AIN SPI - LMP90080 수신 버퍼 */
static uint8_t spi3RxBuffer[10]; /*!< EMP SPI - SY7T609 수신 버퍼 */
bool flag_spi1RxComplete = false; /*!< AIN SPI - LMP90080 수신 완료 플래그, false로 변경해야 수신 가능 */
bool flag_spi3RxComplete = false; /*!< EMP SPI - SY7T609 수신 완료 플래그, false로 변경해야 수신 가능 */

void AIO_Init(void)
{
}

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

void LMP90080_WriteReg(uint8_t regNum, uint16_t regData)
{
  uint8_t spiData[3] = {regNum, (uint8_t)((regData >> 8U) & 0xFFU), (uint8_t)(regData & 0xFFU)};
  HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit_IT(&hspi1, spiData, 3U);
}

void LMP90080_ReadReg(uint8_t regNum)
{
  if(flag_spi1RxComplete == false)
  {
    spi1RxBuffer[0] = 0x10; /* 0x10: Write Address, 0x90: Read Address */
    spi1RxBuffer[1] = (regNum >> 4) & 0x0F;
    spi1RxBuffer[2] = 0x80; /* [7] 0: Write, 1: Read */
    spi1RxBuffer[2]	|= (2 << 5); /* 2Byte Read */
    spi1RxBuffer[2]	|= (regNum & 0x0F);
    HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Receive_IT(&hspi1, spi1RxBuffer, 5); /* [0] : 레지스터, [1] : 레지스터 값 */
  }
}

void LMP90080_Test(void)
{
	LMP90080_ReadReg(0x24);
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
  if(flag_spi3RxComplete == false)
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
    flag_spi1RxComplete = true;
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
  if(flag_spi1RxComplete == true) /* AIN - LMP90080 데이터터 수신 완료 */
  {
	  printf("SPI1 : %x %x %x %x %x",spi1RxBuffer[0],spi1RxBuffer[1],spi1RxBuffer[2],spi1RxBuffer[3],spi1RxBuffer[4]);
    flag_spi1RxComplete = false;
    /* ADC 값 저장 - 추 후 작성 */
  }

  if(flag_spi3RxComplete == true) /* AIN - LMP90080 데이터터 수신 완료 */
  {
	  printf("SPI3 : %x %x %x %x %x",spi3RxBuffer[0],spi3RxBuffer[1],spi3RxBuffer[2],spi3RxBuffer[3],spi3RxBuffer[4]);
	  sendMessageToRS232(MSGCMD_RESPONSE_WATMETER_VAL, spi3RxBuffer, 5);
    flag_spi3RxComplete = false;
    spi3RxBuffer[2] = 0;
    /* ADC 값 저장 - 추 후 작성 */
  }
}
