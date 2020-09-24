/**
  ******************************************************************************
  * @file    uart.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   UART 제어
  * @details

  */

#include <stdio.h>
#include "uart.h"
#include "usart.h"

/** @defgroup UART UART 제어 함수
  * @brief UART 제어 및 링 버퍼
  * @{
  */

static uartFIFO_TypeDef uart1Buffer; /*!< UART1 링 버퍼 구조체 - RS232 */
static uartFIFO_TypeDef uart2Buffer; /*!< UART2 링 버퍼 구조체 - RS485 */
static uartFIFO_TypeDef uart4Buffer; /*!< UART4 링 버퍼 구조체 - Raspberry Pi */

static ErrorStatus putByteToBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t ch); /*!< 버퍼에 1Byte 쓰기 */
static void initBuffer(volatile uartFIFO_TypeDef *buffer);

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
 * @brief UART 관련 설정
 * 
 */
void Uart_Init(void)
{
  initBuffer(&uart1Buffer);
  initBuffer(&uart2Buffer);
  initBuffer(&uart4Buffer);
  (void)HAL_UART_Receive_DMA(&huart1, (uint8_t *)&uart1Buffer.ch, 1U);
  (void)HAL_UART_Receive_DMA(&huart2, (uint8_t *)&uart2Buffer.ch, 1U);
  (void)HAL_UART_Receive_DMA(&huart4, (uint8_t *)&uart4Buffer.ch, 1U);
}

/**
  * @brief  UART RX 인터럽트
  * 
  * @param huart
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if (huart->Instance == USART1) /* RS232 */
  {
    (void)putByteToBuffer(&uart1Buffer, uart1Buffer.ch);
    (void)HAL_UART_Receive_DMA(huart, (uint8_t *)&uart1Buffer.ch, 1U);
  }
  if (huart->Instance == USART2) /* RS485 */
  {
    (void)putByteToBuffer(&uart2Buffer, uart2Buffer.ch);
    (void)HAL_UART_Receive_DMA(huart, (uint8_t *)&uart2Buffer.ch, 1U);
  }
  if (huart->Instance == UART4) /* Raspberry Pi */
  {
    (void)putByteToBuffer(&uart4Buffer, uart4Buffer.ch);
    (void)HAL_UART_Receive_DMA(huart, (uint8_t *)&uart4Buffer.ch, 1U);
  }
}

/**
 * @brief UART 에러 발생 인터럽트
 * 
 * @param huart 
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->ErrorCode == HAL_UART_ERROR_ORE)
  {
    /* Overrun error 처리 */
  }
}

/**
 * @brief UART 수신 버퍼 초기화
 * 
 * @param buffer 
 */
static void initBuffer(volatile uartFIFO_TypeDef *buffer)
{
  buffer->count = 0U; /* 버퍼에 저장된 데이터 갯수 초기화 */
  buffer->in = 0U;    /* 버퍼 시작 인덱스 초기화*/
  buffer->out = 0U;   /* 버퍼 끝 인덱스 초기화 */
}

/**
 * @brief 버퍼에 1byte 저장
 * 
 * @param buffer: UART 버퍼 구조체 포인터
 * @param ch: 저장 할 Byte 데이터
 */
static ErrorStatus putByteToBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t ch)
{
  ErrorStatus status = ERROR;

  if (buffer->count != UART_BUFFER_SIZE) /* 데이터가 버퍼에 가득 찼으면 ERROR 리턴 */
  {
    buffer->buff[buffer->in] = ch; /* 버퍼에 1Byte 저장 */
    buffer->in++;
    buffer->count++;                    /* 버퍼에 저장된 갯수 1 증가 */
    if (buffer->in == UART_BUFFER_SIZE) /* 시작 인덱스가 버퍼의 끝이면 */
    {
      buffer->in = 0U; /* 시작 인덱스를 0부터 다시 시작 */
    }
    status = SUCCESS;
  }
  else
  {
    status = ERROR;
  }

  return status;
}

/**
 * @brief 버퍼에서 1byte 읽기
 * 
 * @param buffer: UART 버퍼 구조체 포인터
 * @param ch: 가져 갈 Byte 데이터 포인터
 * @return ErrorStatus: 버퍼에 데이터가 없으면 ERROR
 *         @arg SUCCESS, ERROR
 */
ErrorStatus getByteFromBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t *ch)
{
  ErrorStatus status = ERROR;

  if (buffer->count != 0U) /* 버퍼에 데이터가 있으면 */
  {

    *ch = buffer->buff[buffer->out]; /* 버퍼에서 1Byte 읽음 */
    buffer->buff[buffer->out] = 0U;
    buffer->out++;
    buffer->count--;                     /* 버퍼에 저장된 데이터 갯수 1 감소 */
    if (buffer->out == UART_BUFFER_SIZE) /* 끝 인덱스가 버퍼의 끝이면 */
    {
      buffer->out = 0U; /* 끝 인덱스를 0부터 다시 시작 */
    }
    status = SUCCESS;
  }
  else
  {
    status = ERROR;
  }

  return status;
}

/**
  * @}
  */
