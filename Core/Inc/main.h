/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LD_AI2_Pin GPIO_PIN_5
#define LD_AI2_GPIO_Port GPIOE
#define LD_AI1_Pin GPIO_PIN_6
#define LD_AI1_GPIO_Port GPIOE
#define RS485_TX2_Pin GPIO_PIN_2
#define RS485_TX2_GPIO_Port GPIOA
#define RS485_RX2_Pin GPIO_PIN_3
#define RS485_RX2_GPIO_Port GPIOA
#define AIN_NSS_Pin GPIO_PIN_4
#define AIN_NSS_GPIO_Port GPIOA
#define AIN_SCK_Pin GPIO_PIN_5
#define AIN_SCK_GPIO_Port GPIOA
#define AIN_MISO_Pin GPIO_PIN_6
#define AIN_MISO_GPIO_Port GPIOA
#define AIN_MOSI_Pin GPIO_PIN_7
#define AIN_MOSI_GPIO_Port GPIOA
#define DI1_Pin GPIO_PIN_7
#define DI1_GPIO_Port GPIOE
#define DI2_Pin GPIO_PIN_8
#define DI2_GPIO_Port GPIOE
#define DI3_Pin GPIO_PIN_9
#define DI3_GPIO_Port GPIOE
#define DI4_Pin GPIO_PIN_10
#define DI4_GPIO_Port GPIOE
#define LD_DI4_Pin GPIO_PIN_11
#define LD_DI4_GPIO_Port GPIOE
#define LD_DI3_Pin GPIO_PIN_12
#define LD_DI3_GPIO_Port GPIOE
#define LD_DI2_Pin GPIO_PIN_13
#define LD_DI2_GPIO_Port GPIOE
#define LD_DI1_Pin GPIO_PIN_14
#define LD_DI1_GPIO_Port GPIOE
#define LD_RS485RDY_Pin GPIO_PIN_15
#define LD_RS485RDY_GPIO_Port GPIOE
#define PSENSOR_SCL_Pin GPIO_PIN_10
#define PSENSOR_SCL_GPIO_Port GPIOB
#define PSENSOR_SDA_Pin GPIO_PIN_11
#define PSENSOR_SDA_GPIO_Port GPIOB
#define LORA_NSS_Pin GPIO_PIN_12
#define LORA_NSS_GPIO_Port GPIOB
#define LORA_SCK_Pin GPIO_PIN_13
#define LORA_SCK_GPIO_Port GPIOB
#define LORA_MISO_Pin GPIO_PIN_14
#define LORA_MISO_GPIO_Port GPIOB
#define LORA_MOSI_Pin GPIO_PIN_15
#define LORA_MOSI_GPIO_Port GPIOB
#define LORA_RFC_Pin GPIO_PIN_8
#define LORA_RFC_GPIO_Port GPIOD
#define LORA_DIO4_Pin GPIO_PIN_9
#define LORA_DIO4_GPIO_Port GPIOD
#define LORA_DIO3_Pin GPIO_PIN_10
#define LORA_DIO3_GPIO_Port GPIOD
#define LORA_DIO2_Pin GPIO_PIN_11
#define LORA_DIO2_GPIO_Port GPIOD
#define LORA_DIO1_Pin GPIO_PIN_12
#define LORA_DIO1_GPIO_Port GPIOD
#define LORA_DIO0_Pin GPIO_PIN_13
#define LORA_DIO0_GPIO_Port GPIOD
#define LORA_RESET__Pin GPIO_PIN_14
#define LORA_RESET__GPIO_Port GPIOD
#define LORA_EN_Pin GPIO_PIN_15
#define LORA_EN_GPIO_Port GPIOD
#define LD_RS232RDY_Pin GPIO_PIN_8
#define LD_RS232RDY_GPIO_Port GPIOA
#define RS232_TX1_Pin GPIO_PIN_9
#define RS232_TX1_GPIO_Port GPIOA
#define RS232_RX1_Pin GPIO_PIN_10
#define RS232_RX1_GPIO_Port GPIOA
#define AO_NSS_Pin GPIO_PIN_15
#define AO_NSS_GPIO_Port GPIOA
#define RASPI_TX4_Pin GPIO_PIN_10
#define RASPI_TX4_GPIO_Port GPIOC
#define RASPI_RX4_Pin GPIO_PIN_11
#define RASPI_RX4_GPIO_Port GPIOC
#define LD_AO_Pin GPIO_PIN_1
#define LD_AO_GPIO_Port GPIOD
#define LD_DO2_Pin GPIO_PIN_3
#define LD_DO2_GPIO_Port GPIOD
#define LD_DO1_Pin GPIO_PIN_4
#define LD_DO1_GPIO_Port GPIOD
#define DO2_Pin GPIO_PIN_5
#define DO2_GPIO_Port GPIOD
#define DO1_Pin GPIO_PIN_6
#define DO1_GPIO_Port GPIOD
#define AO_SCK_Pin GPIO_PIN_3
#define AO_SCK_GPIO_Port GPIOB
#define AO_SDI_Pin GPIO_PIN_5
#define AO_SDI_GPIO_Port GPIOB
#define RTC_SCK_Pin GPIO_PIN_6
#define RTC_SCK_GPIO_Port GPIOB
#define RTC_SDA_Pin GPIO_PIN_7
#define RTC_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
