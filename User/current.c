/**
  ******************************************************************************
  * @file    current.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   전류 측정
  * @details

  */

#include "current.h"
#include "spi.h"

uint8_t spi3RxBuffer[10];                /*!< EMP SPI - SY7T609 수신 버퍼 */
static bool flag_spi3RxComplete = false; /*!< EMP SPI - SY7T609 수신 완료 플래그, false로 변경해야 수신 가능 */


static void SY7T609_WriteRegSingle(uint8_t regNum, uint32_t regData);
static uint32_t SY7T609_ReadRegSingle(uint8_t regNum);



/**
 * @brief EMP Register 쓰기
 * 
 * @param regNum:
 * @param regData:
 */
void SY7T609_WriteReg(uint8_t regNum, uint32_t regData)
{
	if(regNum < 0x40U)
	{
		SY7T609_WriteRegSingle(regNum, regData);
	}
	else
	{
		SY7T609_WriteRegSingle(0x09, regData);
		SY7T609_WriteRegSingle(0x0A, regNum | 0xFF0000);
	}
}

uint32_t SY7T609_ReadReg(uint8_t regNum)
{
	if(regNum < 0x40U)
	{
		return SY7T609_ReadRegSingle(regNum);
	}
	else
	{
		SY7T609_WriteRegSingle(0x0B, regNum);
		return SY7T609_ReadRegSingle(0x0C);
	}
}

static void SY7T609_WriteRegSingle(uint8_t regNum, uint32_t regData)
{
	uint8_t spiTxBuffer[5] = {0x01, (regNum << 2) + 0x02, (regData >> 16)&0xFF, (regData >> 8)&0xFF, (regData >> 0)&0xFF};
	HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, spiTxBuffer, 5U, 0xFFFF);
	HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
}

static uint32_t SY7T609_ReadRegSingle(uint8_t regNum)
{
	uint32_t spiRxData = 0;
	uint8_t spiRxBuffer[5] = {0x01, (regNum << 2) + 0x00, 0, 0, 0};

	HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_RESET);
	if(HAL_SPI_Receive(&hspi3, spiRxBuffer, 5U, 0xFFFF) == HAL_OK) /* [0] : 레지스터, [1] : 레지스터 값 */
	{
		spiRxData = (spiRxBuffer[2]<<16) | (spiRxBuffer[3]<<8) | (spiRxBuffer[4]);
	}
	HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);

	return spiRxData;
}

/**
 * @brief SPI RX 인터럽트
 * 
 * @param hspi 
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
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
  if (hspi->Instance == SPI3) /* EMP - SY7T609 */
  {
    HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
  }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
}



float Current_Read(int8_t No)
{
  float currentValue = 0.0f;
  return currentValue;
}
