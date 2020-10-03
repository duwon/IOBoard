/**
  ******************************************************************************
  * @file    aio.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   Analog Input 제어
  * @details

  */

#include <string.h>
#include "aio.h"
#include "tim.h"
#include "spi.h"


static uint8_t spi1RxBuffer[10]; /*!< AIN SPI - LMP90080 수신 버퍼 */
bool flag_spi1RxComplete = false; /*!< AIN SPI - LMP90080 수신 완료 플래그, false로 변경해야 수신 가능 */

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
    spi1RxBuffer[0] = regNum;
    HAL_GPIO_WritePin(AIN_NSS_GPIO_Port, AIN_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Receive_IT(&hspi1, spi1RxBuffer, 2); /* [0] : 레지스터, [1] : 레지스터 값 */
  }
}

/**
 * @brief DAC 제어
 * 
 * @param OPMode: DAC 동작 모드
 *    @arg DAC_NORMAL, DAC_1K, DAC_100K, DAC_HIGH_Z
 * @param Data: DAC 값
 */
void DAC8551_WriteReg(DAC_MODE_TypeDef OPMode, uint16_t Data)
{
  uint8_t spiData[3] = {(uint8_t)OPMode, (uint8_t)((Data >> 8U) & 0xFFU), (uint8_t)(Data & 0xFFU)};
  HAL_GPIO_WritePin(AO_NSS_GPIO_Port, AO_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit_IT(&hspi3, spiData, 3U);
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
  if (hspi->Instance == SPI3) /* AO - DAC8551 */
  {
    HAL_GPIO_WritePin(AO_NSS_GPIO_Port, AO_NSS_Pin, GPIO_PIN_SET);
  }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
}

/**
 * @brief 100us 인터럽트. Analog input 데이터 저장
 * 
 */
void AIN_TIM_PeriodElapsedCallback(void)
{
  if(flag_spi1RxComplete == true) /* AIN - LMP90080 데이터터 수신 완료 */
  {
    flag_spi1RxComplete = false;
    /* ADC 값 저장 - 추 후 작성 */
  }
}
