/**
  ******************************************************************************
  * @file    uart.c
  * @author  정두원
  * @date    2020-04-10
  * @brief   UART 제어
  * @details

  */

#include <sdtio.h>
#include "usart.h"

/* printf IO 사용을 위한 설정 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

/**
  * @brief  UART RX 인터럽트
  * @param huart
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if (huart->Instance == USART3)
  { /* RS485 */
    putByteToBuffer(&uart3Buffer, uart3Buffer.ch);
    HAL_UART_Receive_IT(huart, (uint8_t *)&uart3Buffer.ch, 1);
  }
  if (huart->Instance == USART1)
  { /* Debug */
    putByteToBuffer(&uart1Buffer, uart1Buffer.ch);
    HAL_UART_Receive_DMA(huart, (uint8_t *)&uart1Buffer.ch, 1);
  }
}

/**
 * @brief UART 에러 발생 인터럽트
 * @param huart 
 * @retval None
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->ErrorCode == HAL_UART_ERROR_ORE)
  {
    /* Overrun error 처리 */
  }
}